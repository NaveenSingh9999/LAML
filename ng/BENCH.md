# LAML v4.1 Benchmarks — measured, no claims without logs

Hardware: Termux on Android (phone CPU), Python-3.14 stdlib clients.
`wrk` is not available here, so HTTP numbers use `bench/bench_http.py`
(keep-alive, N threads x R reqs). Re-run on server hardware with `wrk`
before quoting 5k-class numbers — see §5.

Targets: `make bench-http bench-ws bench-churn bench-slow`

## 1. HTTP keep-alive (`bench_http_server.lm`, `return "ok"`)

| clients | reqs | errs | rps | p50 | p99 | max |
|---|---|---|---|---|---|---|
| 1 | 200 | 0 | ~330 | 2.4ms | ~10ms | ~25ms |
| 20 | 2000 | 0 | 516–658 | ~24–32ms | ~90–164ms | ~300ms |
| 50 | 2000 | 0 | ~650 | ~42ms | ~246ms | ~430ms |

Server-side view during 20-conn run (`/__stats`): handler p50/p99 = 1ms
bucket (sub-ms actual), `dropped=0`, 2000/2000 served.

Reading: throughput is flat (~600 rps) while client latency grows with
contention — the cost is loopback + Python client + phone scheduling,
not the interpreter (handlers stay sub-ms). 5k rps needs server hardware
+ `wrk`; the server side has headroom (0 drops, sub-ms handlers).

## 2. WS fan-out (`bench_chat.lm` relay, 50 clients x 3 rounds)

150 sends -> 7350 receives (x49 fan-out): **100% delivered**,
p50 ~1.3ms, p99 ~75–90ms, 0 drops after byte-cap fix.
(An earlier run with a 64-entry queue cap dropped 6% of benign bursts;
the cap is now the spec'd 256KB per fd.)

## 3. Churn (1000 connect/close WS)

`rss +96–104KB` total (~100B/conn noise), `alive=yes`, no fd leak.
Old `net*` blocking API untouched; legacy scripts pass.

## 4. Idle footprint

| conns | server RSS |
|---|---|
| 500 WS | 6.1MB (~12KB/conn all-in) |
| 2000 WS | 13.3MB (~6.6KB/conn all-in) |

Linear projection: 5k idle ≈ 33MB — inside the 100MB gate with margin.
(Kernel buffers tuned to 16KB RCV/SNDBUF; defaults alone would eat 1GB.)

## 5. Slow client (1 blackhole + 10 fast, 200 x 4KB broadcasts)

Fast clients got **2000/2000**, p50 0.2ms / p99 44.5ms.
Blackhole's queue hit the 256KB cap → `dropped=118`, room unaffected,
server alive. Matches spec: bounded queue, drop-don't-OOM.

## 6. Drain

SIGTERM with 5 open WS conns: exit 0 in <1s (no 6s `_Exit` fallback hit).

## 7. What would raise the ceiling (not done, measured next)

- `wrk -t4 -c500` on x86 server box (Python GIL + phone CPU dominate here).
- `TCP_QUICKACK` showed no win on loopback (kept: harmless, helps real NICs).
- Self-pipe loop wakeup: keeps response latency off the 50ms poll quantum.
- `io_uring` only if profiles show syscall dominance (not the case here).

## 8. Reproduce

```bash
make bench-http   # 20 conns x 100 reqs, expect 0 errs
make bench-ws     # 50 clients, expect got=7350
make bench-churn  # 1000 churn, expect delta_kb < 500
make bench-slow   # expect got=2000 + drops > 0
curl localhost:18090/__stats   # with {stats:true}
```
