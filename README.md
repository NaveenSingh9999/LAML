# LAML v4.1.0 — dynamic language for realtime servers

LAML (Low Abstraction Machine Language) is a minimal dynamic language in C++20 for writing **realtime servers** — WebSocket, SSE, relay, pub/sub, game rooms, signaling — plus everyday scripting. Checked arithmetic, first-class functions, error-as-values, and supervised OS-thread concurrency.

## Install

```bash
# Universal installer (auto-detects OS/arch, prebuilt binary or source build)
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/install.sh | bash

# Or grab a prebuilt binary from the v4.1.0 release
wget https://github.com/NaveenSingh9999/LAML/releases/download/v4.1.0/laml-linux-aarch64
chmod +x laml-linux-aarch64 && sudo mv laml-linux-aarch64 /usr/local/bin/laml

# Verify
laml version        # LAML v4.1.0 — dynamic language for realtime servers (C++20)
laml --help
```

Assets per arch: `laml-linux-aarch64` + `laml-v4.1.0.tar.gz` today (more as CI/maintainers add them); other platforms install from source via the same command — same version, native binary.

## What's New in v4.1.0

| Area | Feature |
|------|---------|
| **Realtime** | `serve/on/send/broadcast` — WS (RFC6455) + HTTP/1.1 + SSE on one port, rooms, presence, 54s heartbeat |
| **Dynamic DX** | `jsonParse/jsonStringify`, `setTimeout/setInterval`, `async/waitFor`, `reload()` without dropping connections, `stats()` + `/__stats` |
| **Stdlib** | Math (`abs/min/max/floor/ceil/sqrt/pow/randInt`), arrays (`pop/remove/sort/reverse/join/slice`), strings (`upper/lower/replace/startsWith/endsWith/repeat`), `keys/has/string/fileExists/fileDelete/appendFile/assert` |
| **Core** | Float `<=/>=/%`, full mixed int/float comparisons, `obj["key"]` indexing, `nil` literal, `closc :priority`, graceful SIGTERM drain |
| **Measured** | 7350/7350 WS fan-out delivered, 2000 idle WS at 13MB, 1000-conn churn +100KB (`ng/BENCH.md`) |

## Quick Start

```bash
laml run program.lm
laml repl
```

```laml
say "Hello, World!"
```

## 10-line chat relay

```laml
on("connect", func(fd, path) { joinRoom(fd, "lobby") })
on("message", func(fd, msg) { broadcast("lobby", msg, fd) })
on("close", func(fd) { leaveAll(fd) })
serve(8080, {ws: true, stats: true})
```

## Language Features

- **Values**: `nil`, `int`, `float`, `string`, `bool`, `array`, `func`, `obj`, `error`
- **Variables**: `val` (immutable), `let` (mutable)
- **Operators**: `+ - * / %`, `== != < > <= >=`, `&& || !` (float + mixed-type complete)
- **Control Flow**: `if/else`, `while`, `loop`, `loop x in a to b`, `for x in coll`, `break/continue`
- **Functions**: First-class with closures, `func name(params) { body }`, `return`
- **Arrays**: Heterogeneous, bounds-checked, `[]` indexing, `sort/pop/join/slice`
- **Objects**: `{k: v}` literals, `.prop` and `["prop"]` access, `keys()/has()`
- **Strings**: Escapes, `len()`, indexing, full case/search/replace set
- **Errors**: Error-as-values, `try(func)` → `[ok, val]`, `assert(cond, msg)`
- **Concurrency**: `closc name :prio { }` supervisors, `async/waitFor` pool tasks
- **Realtime**: `serve/on/send/broadcast/joinRoom/members`, rooms + timers + JSON
- **Comments**: `~` line comments

## Architecture

```
┌─────────────┐
│  Lexer      │  Tokenizes source → Token stream
├─────────────┤
│  Parser     │  Pratt parser → AST
├─────────────┤
│  Evaluator  │  Tree-walking interpreter, closures, env chains
├─────────────┤
│  RT loop    │  Non-blocking event loop (epoll/poll): HTTP/WS/SSE/UDP
├─────────────┤
│  Workers    │  Scheduler pool runs LAML handlers, never blocks IO
├─────────────┤
│  Bus        │  Sharded rooms, timer wheel, JSON, metrics
└─────────────┘
```

## Project Structure

```
LAML/
├── ng/                    # C++20 implementation
│   ├── src/               # Lexer/parser/evaluator + rt_* realtime core
│   ├── bench/             # Bench harness (make bench-*)
│   ├── BENCH.md           # Measured numbers + methodology
│   ├── Makefile           # Build system
│   └── examples/          # LAML example programs (rt_chat/game/signal/sse)
├── docs/                  # Documentation website (learn/setup/features/changelog)
├── examples/              # Example programs
├── vscode-extension/      # VS Code syntax highlighting
├── installers/            # Platform installer scripts
├── install.sh             # Universal installer (v4.1.0)
├── laml                   # Prebuilt binary (this arch)
└── LAML-NG-GUIDE.md       # Complete language guide
```

## Learn More

- [Learn](https://naveensingh9999.github.io/LAML/learn.html) — Interactive tutorial (realtime + stdlib lessons)
- [Features](https://naveensingh9999.github.io/LAML/features.html) — Feature tour + language comparison
- [Setup](https://naveensingh9999.github.io/LAML/setup.html) — Install + verify + VS Code
- [Changelog](https://naveensingh9999.github.io/LAML/changelog.html) — What changed
- [GitHub](https://github.com/NaveenSingh9999/LAML) — Source code
