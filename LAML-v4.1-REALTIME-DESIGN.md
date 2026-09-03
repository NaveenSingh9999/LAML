# LAML v4.1 — Language for Realtime Servers (Spec, language-first)

> v4.1 after v4.0.0 | Goal: easiest + fastest dynamic language to *write* realtime servers | Target: 5000 req/s on 1GB, p99 <20ms | Assumed decisions (approved plan): `closc` per-fd forbidden + warn over 64 live; TLS via proxy in 4.1.

## 0. Vision (one line)

LAML stays a **dynamic programming language**, not a server program. v4.1 gives programmers 15 verbs to write any realtime server in 10-20 lines. C++20 does bare metal underneath — no pointers, no registers, no assembly in LAML code.

Judged by: **how easy is user code + how fast does user code run**, not how fast our demo is.

## 1. Principles

1. No new syntax in 4.1 — builtins only, `ast.h` frozen.
2. `fd:int`, `msg:string|obj`, `room:string`. Never expose pointers.
3. `closc` = user supervisors (`keeper/janitor/matchmaker`). Same keyword, clarified role, priority honored.
4. `scheduler` = user `async/waitFor`. No second pool.
5. Every remote event isolated via `try` — `Error` logs + optional `close`, never kills loop.
6. `val` = shareable RO config, `let` = worker-local mutable.
7. Loop never interprets, workers never block on IO. Slow user handler delays one worker, never accept.

## 2. Where v4 stands (ground truth)

| Layer | File | Reality |
|---|---|---|
| Entry | `ng/src/main.cpp:39-68` | `readFile -> Lexer -> Parser -> Evaluator`, then `waitAll` |
| Lexer/Parser | `ng/src/lexer.cpp:109-224`, `ng/src/parser.cpp:302-360` | ~15 keywords, Pratt, `Program/If/While/Loop/Func/Call/Index/Dot/Closc` |
| Value/Env | `ng/src/value.h:13-81`, `ng/src/env.h:11-67` | dynamic `Value` (copy-heavy), chained `Env` + per-level `mutex` |
| Eval | `ng/src/evaluator.cpp:21-163,513,532-568` | tree-walker, `new Env` per call/block, checked arithmetic, bounds check, errors-as-values |
| Net | `ng/src/net_builtins.cpp:29-215` | blocking TCP/UDP, `SO_REUSEADDR` only, `listen(128)`, `inet_pton` IPv4-only |
| Closc | `ng/src/closc.cpp:19-29`, `evaluator.cpp:136-158` | 1 OS thread per `closc`, priority hardcoded `5`, `stop` never checked |
| Scheduler | `ng/src/scheduler.cpp:4-71` | pool sized to cores, 100ms `wait_for` poll, per-task mutex/cv, unreachable from LAML |
| Server demo | `ng/server.lm:1-185` | HTTP via `socat + readLine/say` |
| Str/crypto | `ng/src/str_builtins.cpp:1-261` | `slice/split/trim/chr/sha256/base64/randomBytes` — reuse for WS/auth |

Blockers for 5k/1GB: per-`Ident` `Value` copy (`env.h:22-31`), per-call `new Env`, 100ms scheduler poll, blocking net, 8MB thread stacks, kernel buffers at default 200KB/socket.

## 3. Language spec (frozen for 4.1)

### 3.1 New builtins (entire delta)

```
serve(port, {ws:true, http:true, udp:9000, workers:4}) -> nil|error
on("connect|message|request|close|timeout|drain", func) -> nil
send(fd, msg) -> bytes|error
broadcast(room, msg[, exceptFd]) -> count
joinRoom(fd,room) / leaveRoom(fd,room) / leaveAll(fd) / members(room) -> array
setTimeout(func,ms)->id / setInterval(func,ms)->id / cancelTimer(id)
async(func)->id / waitFor(id)
jsonParse(str)->val / jsonStringify(val)->str
sseSend(fd, data) / close(fd[, code]) / stats() / reload(path)
```

No parser change. `serve/on/...` are `Builtin` values like `say/len`.

### 3.2 Semantics (normative)

- **Dispatch:** loop sequences events per-fd FIFO; workers run closures in order per fd, parallel across fds. Single writer per fd in loop preserves send order.
- **Handlers:** `on(evt, func)` caches `FuncData{params, body*, closure}`. Called as `func(fd, payload)` for connect/message/close, `func(req)` for request, `func(tag)` for timeout. Arity mismatch => `Error` value, not crash.
- **Isolation:** each dispatch wrapped as `try(handler)`. `Error` => `say("[rt] "+err)` + continue; for message/request also optional `close(fd)` if handler returns `error`. Loop lives.
- **Rooms:** `join` = subscribe. `broadcast` fans out in C++, returns delivered count. Slow fd (>256KB queued or `bufferedAmount>1MB`) dropped with `close(1013)` + `dropped++`, room continues.
- **Request:** `req = {method,path,query,headers:obj,body,fd}`. Return `{status:200, body:"hi", headers:{...}}` or `{sse:true}` to hold open for `sseSend`.
- **Timers:** wheel-backed, `setTimeout` fires once as `on("timeout")` with stored tag/closure, `setInterval` repeats until `cancelTimer`.
- **`closc` contract:** `closc keeper { while true { ... sleep(5000) } }` spawns one OS thread at declaration with `Env` snapshot. Allowed 0-64 live (warn over 64, error over 256 to protect 1GB). Must call `sleep()` or block on `waitFor` periodically so `requestStop()` drains. Priority param honored: keeper/ticker high, logger low.
- **`async` contract:** `async(func)` submits to scheduler pool, returns id string. `waitFor(id)` blocks *worker* (not loop) until done. For physics, JSON prep, fan-out prep.
- **`reload` contract:** re-`import`s file, parses to new AST (kept alive like `sys_builtins.cpp:75-94` `gImportedAsts`), atomically swaps `HandlerTable`. Existing conns keep fd/room, next event uses new handlers. Parse error => old handlers stay, return `error`.
- **`val/let` in handlers:** `val ROOM="lobby"` RO shares safely; `let n=0` inside handler is per-invocation Env copy. Cross-handler mutable state must go via rooms/messages, not globals (documented race guard).

### 3.3 Types_payloads

- WS text => LAML `string`. Binary => `string` bytes (`type()=="string"`, `len()` = bytes). Control frames (ping/pong/close) handled in C++, never surface except `on("close")`.
- `jsonParse`: `null->nil`, ints that fit => `int` else `float`, escapes incl `\uXXXX`, caps 1MB doc / 128 depth => `error` on breach. `jsonStringify`: depth cap, binary strings raw.
- `stats()` => `{conns,rooms,rps,p50Ms,p99Ms,rssMB,dropped}`.

## 4. Reused best features (how each maps)

- `closc` => supervisors (`lobbyKeeper`, `janitor`, `matchmaker`). Fix: honor priority (not hardcoded 5), stop-aware `sleep`.
- `scheduler` => handler + `async` workers. Fix: replace 100ms poll with eventfd ring, reuse pool (no second pool).
- `func` closures => `on()` handlers + factories (`makeRoomHandler(room)` returns closure).
- `try` + errors => isolation wrapper for every dispatch.
- `import` => `reload` foundation (AST lifetime already handled).
- Checked arithmetic + bounds check (`evaluator.cpp:532-568`, index guards) => safe untrusted client numbers (positions, indexes) yield `error`, not wrap/crash.
- `sha256`+`base64` => WS accept key. `randomBytes`+`hexEncode` => auth tokens. `split/trim` => path/query parsing fallback.
- `say/readFile/writeFile/append/args/env` => logging, static serving (`readFile("www"+path)`), room logs, `serve(env("PORT"))` config.
- Legacy `net*` stay as `blocking-legacy` for tools; `udp*` stay for discovery + `serve(udp:)` fast path uses nonblock bucket underneath.

## 5. Canonical user programs (acceptance DX)

Each must run unmodified on Linux + Termux, each <=30 lines:

```laml
~ rt_chat.lm (10 lines)
serve(8080, {ws: true})
on("connect", func(fd, path) { joinRoom(fd, "lobby") })
on("message", func(fd, msg) { broadcast("lobby", msg, fd) })
```

```laml
~ rt_game.lm
serve(8081, {ws: true})
on("message", func(fd, msg) {
  val m = jsonParse(msg)
  if type(m) == "error" { return }
  broadcast(m["room"], msg, fd)
})
```

```laml
~ rt_signal.lm (WebRTC/STUN-style)
serve(8082, {ws: true, udp: 9000})
on("message", func(fd, msg) {
  val m = jsonParse(msg)
  if m["to"] != nil { send(toInt(m["to"]), msg) } else { broadcast("hall", msg) }
})
```

```laml
~ rt_sse.lm
serve(8080, {http: true})
on("request", func(req) {
  if req["path"] == "/tick" { return {sse: true} }
  return {body: readFile("www/index.html")}
})
setInterval(func() { broadcast("tickers", jsonStringify({t: clock()})) }, 1000)
```

```laml
~ rt_closc.lm (supervisors + isolation + RO config)
val ROOM = "lobby"
closc keeper {
  while true {
    val r = try(func() { broadcast(ROOM, jsonStringify({t: clock(), n: len(members(ROOM))})) })
    if r[0] == false { say("keeper err " + r[1]) }
    sleep(5000)
  }
}
closc janitor {
  while true { sleep(30000) say("conns " + stats()["conns"]) }
}
serve(8080, {ws: true})
on("connect", func(fd, path) { joinRoom(fd, ROOM) })
on("message", func(fd, msg) { broadcast(ROOM, msg, fd) })
```

## 6. Runtime support (minimal, serves language)

- `rt_loop`: nonblock, `epoll` Linux / `poll` fallback, `REUSEADDR+REUSEPORT`, backlog 4096, `NODELAY`, 16KB RCV/SNDBUF, `CORK` for HTTP. 1 loop pinned, N=scheduler workers. `Conn` fixed 512B in `vector` by fd.
- `rt_http`: incremental, 16KB header cap / 32 headers, keep-alive, upgrade detect.
- `rt_ws`: handshake, masked decode, FIN/0x1/0x2/0x8/0x9/0xA, 1MB cap => 1009, auto-pong, reassemble.
- `rt_room`: 16 shards, `room->vector<int>` + `fd->rooms` inline, O(1).
- `rt_timer`: 1024x250ms wheel, tickless when idle, 54s±5s heartbeat, 60s dead-free.
- `rt_json`: single-pass, caps above.
- `rt_event`: MPSC 64k in, SPSC per worker out, `eventfd` wake. Full => drop newest Request (503 count), never Close.
- `rt_bridge`: `HandlerTable` atomic swap, `Func*` cache, `Value(string&&)` move overload, Env pool (clear keeps buckets), `val` shared RO.
- `rt_metrics`: `tRecv/tDone` histogram 0-500ms, `/__stats` optional.
- Fixes: `sleep` interruptible, `say` queued (no interleaved `cout`), `netRead` 64KB cap in legacy path, `FD_CLOEXEC` everywhere.

Memory: 512B state + 2KB avg queue + shared 64KBx8 slab + 512KB stacks (not 8MB) + 20MB live strings => ~22MB idle, ~50-150MB at 5k rps. Kernel buffers tuned or math fails — document `sysctl`.

## 7. Milestones (each shippable, language-led)

**M1 Handlers+HTTP:** `rt_event,rt_loop,rt_http,rt_bridge(serve/on-request/send/close)`. Tests: LAML asserts on `req` shape + return shape, split/keep-alive vectors, `wrk -t4 -c200 -d30s` 5k gate, old tests green. Exit: `serve()` static file beats `socat` 10x.
**M2 WS+Rooms:** `rt_ws,rt_room,rt_timer`, heartbeat. Tests: Autobahn text/binary/ping/close/frag-reject, 5k idle RSS, churn fd check, 1->5k fan-out log. Exit: `rt_chat` <100MB.
**M3 Live DX:** `rt_json,reload,sse,udp,stats,async,closc-priority`. Tests: JSON fuzz/depth bomb, reload keeps fds + swaps version, SSE curl, UDP echo, `async` ordering. Exit: all 5 canonical scripts run.
**M4 Harden:** slow-client, drain, stats endpoint, `make bench`, docs. Tests: 30s sustained + p99 + RSS in `BENCH.md`, Termux smoke. Exit: §8 gates, tag v4.1.0.

## 8. Gates for v4.1.0 tag

- [ ] All 15 verbs work from LAML scripts (no C++ test cheats)
- [ ] 5000 rps HTTP 30s + 5000 idle WS <100MB, logs attached
- [ ] `mallocs/req==0` steady-state, p99<20ms with fan-out
- [ ] No fd/RSS leak on 10x churn, slow-client isolated, SIGTERM drains (no fast `_Exit`)
- [ ] 5 canonical scripts <=30 lines, run Linux + Termux
- [ ] Old examples pass, legacy net marked blocking

## 9. Bench harness (ships)

```bash
make bench-http  # wrk -t4 -c500 -d30s http://127.0.0.1:8080/small
make bench-ws    # 5k conns, 1msg/s 1KB broadcast -> fanout p50/p99,RSS,CPU
make bench-churn # 5k connect/close x10 -> fd delta==0, RSS delta<5MB
make bench-slow  # 1 blackhole in 5k room -> drops==1, others p99<20ms
```

Record `conns,rps,p50/p99,RSS,drops,CPU,RCV/SNDBUF,stack`. No claim without log.

## 10. Non-goals 4.1 (no break to 4.2)

Native TLS (proxy), HTTP/2, clustering (Redis Streams bridge next), per-message-deflate, `io_uring` (only if profile shows syscall dominance), JIT.

## 11. Risks

| Risk | Mitigation |
|---|---|
| Tree-walker >0.15ms/handler | Cache Func*, pool Env, ban `inspect()` hot path, C++ fan-out |
| Kernel buffers blow 1GB | 16KB tunes + `ss -m` check in bench |
| WS interop | Autobahn + wscat CI, strict mask/UTF-8 |
| Reload race | Atomic table swap + RCU retire |
| `closc` abuse per-fd | Warn >64, error >256, docs supervisor pattern |
