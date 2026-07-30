# LAML v4.0.0 — C++20 Next-Generation Runtime

LAML (Low Abstraction Machine Language) is a minimal scripting language implemented in C++20 with an LLVM backend, checked arithmetic, first-class functions, and OS-thread concurrency.

## What's New in v4.0.0

| Area | Feature |
|------|---------|
| **Runtime** | Pure C++20 implementation (Go removed) |
| **Safety** | Checked arithmetic (overflow/div-by-zero → error), bounds-checked arrays |
| **Concurrency** | OS-thread based CLOSC system, dataflow scheduler |
| **Performance** | LLVM JIT skeleton, efficient tree-walking interpreter |
| **Builtins** | `readLine()`, `readFile()`, `type()`, `len()`, `sleep()`, `clock()` |
| **Server** | HTTP/1.0 file server written in pure LAML |
| **Errors** | Error-as-values model with `type()` checking |

## Quick Start

```bash
# Run a LAML program
laml run program.lm

# Run the built-in HTTP file server
socat TCP-LISTEN:8080,fork EXEC:"./laml run server.lm"

# Check version
laml --help
```

## Hello World

```laml
say "Hello, World!"
```

## Language Features

- **Values**: `nil`, `int`, `float`, `string`, `bool`, `array`, `func`
- **Variables**: `val` (immutable binding), `let` (mutable binding)
- **Operators**: Arithmetic (`+`, `-`, `*`, `/`, `%`), comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`), logical (`&&`, `||`, `!`)
- **Control Flow**: `if/else`, `while`, `loop`, range `loop x in start to end`
- **Functions**: First-class with closures, `func name(params) { body }`
- **Arrays**: Heterogeneous, bounds-checked, `[]` indexing
- **Strings**: Escape sequences (`\n`, `\r`, `\t`, `\\`, `\"`), `len()`, indexing by code point
- **Concurrency**: `closc` (OS-thread closures with priority scheduling)
- **Comments**: `~` line comments, `{~ ~}` block comments

## Architecture

```
┌─────────────┐
│  Lexer      │  Tokenizes source → Token stream
├─────────────┤
│  Parser     │  Pratt parser → AST
├─────────────┤
│  Evaluator  │  Tree-walking interpreter, closures, env chains
├─────────────┤
│  Builtins   │  I/O, type introspection, file ops
├─────────────┤
│  Scheduler  │  Dataflow scheduler for CLOSC threads
├─────────────┤
│  LLVM JIT   │  Experimental JIT/AOT compilation (skeleton)
└─────────────┘
```

## Project Structure

```
LAML/
├── ng/                    # C++20 implementation
│   ├── src/               # Source files (10 .cpp, 9 .h)
│   ├── build/             # Compiled objects
│   ├── Makefile           # Build system
│   ├── server.lm          # HTTP/1.0 file server in LAML
│   ├── www/               # Web server document root
│   └── examples/          # LAML example programs
├── docs/                  # Documentation website
├── examples/              # Example programs
├── vscode-extension/      # VS Code syntax highlighting
├── installers/            # Platform installer scripts
├── laml                   # C++20 binary (aarch64)
└── LAML-NG-GUIDE.md       # Complete language guide
```

## Learn More

- [LAML-NG-GUIDE.md](LAML-NG-GUIDE.md) — Complete language reference
- [Learn](https://naveensingh9999.github.io/LAML/learn.html) — Interactive tutorial
- [GitHub](https://github.com/NaveenSingh9999/LAML) — Source code
