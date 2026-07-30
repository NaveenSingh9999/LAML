# LAML-NG: Complete Programming Guide

> **Version:** 1.0 — *"Ultra Simple, C-Powered, LLVM-Backed"*

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Getting Started](#2-getting-started)
3. [Hello, LAML](#3-hello-laml)
4. [Values & Types](#4-values--types)
5. [Variables](#5-variables)
6. [Operators](#6-operators)
7. [Strings](#7-strings)
8. [Control Flow](#8-control-flow)
9. [Functions](#9-functions)
10. [Arrays](#10-arrays)
11. [Comments](#11-comments)
12. [Built-in Functions](#12-built-in-functions)
13. [Blocks & Scope](#13-blocks--scope)
14. [Closures](#14-closures)
15. [Concurrency with CLOSC](#15-concurrency-with-closc)
16. [Error Handling](#16-error-handling)
17. [Intermediate Patterns](#17-intermediate-patterns)
18. [Advanced Topics](#18-advanced-topics)
19. [CLI Reference](#19-cli-reference)
20. [Language Quirks & Limitations](#20-language-quirks--limitations)

---

## 1. Introduction

LAML-NG is a small, expression-oriented scripting language built from scratch in C++20 with an LLVM backend. It is designed to be **ultra simple** — no type annotations, no classes, no exceptions, just values, functions, and a handful of keywords. Despite its simplicity, it has real concurrency support (OS threads via CLOSC), a dataflow scheduler, and checked arithmetic that never crashes.

### Design Philosophy

- **Minimal syntax** — Fewer keywords than Python, no semicolons required
- **Safety first** — Overflow-checked math, bounds-checked arrays, error-as-values
- **Concurrent by default** — CLOSC sections run in their own threads
- **LLVM foundation** — JIT compilation path ready for future optimization

---

## 2. Getting Started

### Prerequisites

- LAML-NG is compiled to a single binary called `laml`
- The binary lives at `LAML/ng/laml`

### Running LAML

```bash
# Run a file
laml run myprogram.lm

# Start the interactive REPL
laml repl

# Short form (omit 'run')
laml myprogram.lm
```

### Hello World

```laml
say "hello world"
```

Save this as `hello.lm` and run:

```bash
laml hello.lm
# Output: hello world
```

---

## 3. Hello, LAML

Let's start with a complete first program:

```laml
~ My first LAML program

say "Welcome to LAML-NG!"

val name = "Alex"
val age = 30

say "Hello, " + name
say "You are " + age + " years old"

if age >= 18 {
    say "You are an adult"
} else {
    say "You are a minor"
}
```

Run it:

```bash
laml hello.lm
```

Output:
```
Welcome to LAML-NG!
Hello, Alex
You are 30 years old
You are an adult
```

### Key Takeaways

- `say` prints values
- `val` declares a variable
- `+` works on strings AND numbers
- `if/else` works as expected
- `~` starts a comment

---

## 4. Values & Types

LAML has 10 value types, but you only need to know about these to start:

### Numbers (Int)

```laml
42          ~ positive
-7          ~ negative
0           ~ zero
999999999999 ~ large (64-bit signed)
```

All integers are 64-bit signed (`int64_t`). Arithmetic is **overflow-checked** — if a calculation overflows, you get a runtime error instead of silent wraparound.

### Floating Point (Float)

```laml
3.14
-0.5
100.0
```

Floats are IEEE 754 double-precision.

### Strings

```laml
"hello world"
"LAML-NG"
"42"
```

Strings are double-quoted. No single quotes, no string interpolation.

### Booleans

```laml
true
false
```

### Nil

`nil` represents "no value." There is no way to write `nil` directly in source code — it arises from:

```laml
return;              ~ returns nil
{ }                  ~ empty block evaluates to nil
```

### Checking Types

Use the `type()` builtin:

```laml
say type(42)         ~ "int"
say type(3.14)       ~ "float"
say type("hello")    ~ "string"
say type(true)       ~ "bool"
say type([1,2,3])    ~ "array"
say type(func(x)x)   ~ "func"
```

### Truthiness

When a value is used as a condition (in `if`, `while`, `!`, `&&`, `||`):

| Value                     | Truthy? |
|---------------------------|---------|
| `nil`                     | No      |
| `false`                   | No      |
| `0`                       | No      |
| `0.0`                     | No      |
| `""` (empty string)       | No      |
| `[]` (empty array)        | No      |
| Everything else           | Yes     |

```laml
if 0 { say "no" }           ~ never prints
if 1 { say "yes" }          ~ prints "yes"
if "hello" { say "yes" }    ~ prints "yes" (non-empty string)
if [] { say "no" }          ~ never prints (empty array)
```

---

## 5. Variables

### Declaration

```laml
val x = 10        ~ declare and initialize
let y = 20        ~ also declare and initialize
```

**Important:** `val` and `let` are identical in behavior. Both create a variable in the current scope. Despite the names suggesting immutability (`val`) vs mutability (`let`), **both are mutable** — the language does not enforce single-assignment.

### Reassignment

```laml
val x = 10
x = 20            ~ works fine

let y = 30
y = 40            ~ also works fine
```

### Scope

Variables live in the scope where they are declared. Inner scopes (blocks, functions) can access outer variables but cannot modify them directly (assignment always writes to the **current** scope).

```laml
val outer = "visible everywhere"

{
    val inner = "only visible in this block"
    say outer    ~ works
    say inner    ~ works
}

say outer        ~ works
say inner        ~ ERROR: undefined: inner
```

### Variable Naming

Identifiers start with a letter or underscore, followed by letters, digits, or underscores:

```laml
val myVar = 1
val _private = 2
val camelCase = 3
val snake_case = 4
```

---

## 6. Operators

### Arithmetic

```laml
10 + 20     ~ addition: 30
20 - 10     ~ subtraction: 10
5 * 3       ~ multiplication: 15
20 / 5      ~ division: 4
20 % 3      ~ modulo: 2
```

**Safety:** All integer arithmetic is overflow-checked. Division and modulo by zero return errors.

```laml
say 9223372036854775807 + 1    ~ Runtime error: integer overflow
say 10 / 0                      ~ Runtime error: division by zero
```

### Comparison

```laml
5 == 5      ~ equal: true
5 != 3      ~ not equal: true
5 > 3       ~ greater: true
5 < 3       ~ less: false
5 >= 5      ~ greater or equal: true
5 <= 4      ~ less or equal: false
```

### Logical

```laml
!true           ~ not: false
true && false   ~ and: false (short-circuits)
true || false   ~ or: true (short-circuits)
```

### Operator Precedence

Highest to lowest:

| Precedence | Operators                  |
|-----------|----------------------------|
| 8         | `.` `[` `(`                |
| 5         | `*` `/` `%`                |
| 4         | `+` `-`                    |
| 3         | `==` `!=` `<` `>` `<=` `>=`|
| 2         | `&&` `\|\|`                |
| 1         | `=`                        |
| (prefix)  | `-` `!`                    |

```laml
10 + 20 * 3         ~ 70 (not 90)
(10 + 20) * 3       ~ 90
10 + 20 == 30       ~ true (10+20 is computed first)
!true && false      ~ false (!true is false, short-circuits)
```

### Type Mixing in Operations

```laml
~ Int + Int = Int
10 + 20         ~ 30

~ Float + Float = Float
3.5 + 1.2       ~ 4.7

~ Mixed Int/Float = Float
10 + 3.5        ~ 13.5

~ String + anything = concatenation
"Hello, " + "World!"       ~ "Hello, World!"
"Age: " + 30               ~ "Age: 30"
```

---

## 7. Strings

### Literals

```laml
"Hello, World!"
"Line 1\nLine 2"    ~ \n is a literal newline character
"Quote: \" inside"  ~ escaped quote
"Backslash: \\"     ~ escaped backslash
```

### Concatenation

```laml
"Hello" + " " + "World"    ~ "Hello World"
```

When using `+` with a string and a non-string, the non-string is converted via its string representation:

```laml
"The answer is " + 42      ~ "The answer is 42"
```

### Comparison

```laml
"abc" == "abc"      ~ true
"abc" != "xyz"      ~ true
```

### Length

```laml
len("hello")        ~ 5
len("")              ~ 0
```

---

## 8. Control Flow

### If / Else

```laml
val x = 10

if x > 5 {
    say "big"
}

if x > 20 {
    say "very big"
} else {
    say "not that big"
}
```

The condition is evaluated for truthiness. The branch can be a block or a single expression:

```laml
if x > 5 say "big"       ~ single expression, no braces needed
```

**Note:** Chained `else if` is not supported. Only a single optional `else`.

### While

```laml
val count = 0
while count < 3 {
    say count
    count = count + 1
}
```

Output:
```
0
1
2
```

Loops have a safety cap of **10 million iterations**. If exceeded, an error is returned.

### Loop (General)

`loop` without the range syntax behaves identically to `while`:

```laml
val i = 0
loop i < 5 {
    say i
    i = i + 1
}
```

### Loop (Range)

The range loop iterates a variable from a start value to an end value **inclusive**:

```laml
loop i in 1 to 5 {
    say i
}
```

Output:
```
1
2
3
4
5
```

Both start and end must be integers. The loop variable is scoped to the loop body:

```laml
loop i in 1 to 3 {
    say i        ~ 1, 2, 3
}
say i            ~ ERROR: undefined: i
```

---

## 9. Functions

### Defining Functions

```laml
~ With a block body
func add(a, b) {
    a + b
}

~ With a single-expression body (no braces needed)
func double(x) x * 2

~ Anonymous function (expression form)
val triple = func(x) x * 3
```

### Calling Functions

```laml
say add(3, 4)        ~ 7
say double(10)       ~ 20
say triple(5)        ~ 15
```

Arguments are **eagerly evaluated** (all evaluated before the call) and **must match** the parameter count exactly:

```laml
add(3)                ~ ERROR: expected 2 args, got 1
add(3, 4, 5)          ~ ERROR: expected 2 args, got 3
```

### Return Values

The last expression in the function body is the return value:

```laml
func add(a, b) {
    a + b             ~ this is the return value
}
```

Explicit `return` also works:

```laml
func max(a, b) {
    if a > b {
        return a
    }
    return b
}
```

`return` without a value returns `nil`:

```laml
func noop() {
    return
}
```

### Functions Are Values

Functions are first-class values — you can pass them around:

```laml
func apply(f, x) {
    f(x)
}

val result = apply(func(x) x * 10, 5)
say result            ~ 50
```

### Named vs Anonymous

When `func` has a name, it is automatically bound in the current scope:

```laml
func greet(name) {             ~ binds 'greet'
    say "Hello, " + name
}

val f = func(name) {           ~ anonymous, must assign manually
    say "Hello, " + name
}
```

---

## 10. Arrays

### Creating Arrays

```laml
val empty = []
val numbers = [1, 2, 3, 4, 5]
val mixed = [1, "hello", true, [2, 3]]
```

Arrays are **heterogeneous** — they can hold any value type.

### Accessing Elements

```laml
val arr = [10, 20, 30]
say arr[0]          ~ 10
say arr[1]          ~ 20
say arr[2]          ~ 30
```

Indexing is **zero-based** and **bounds-checked**:

```laml
arr[-1]             ~ ERROR: index out of bounds
arr[100]            ~ ERROR: index out of bounds
```

### Array Length

```laml
say len([1, 2, 3])   ~ 3
```

### Array of Arrays

```laml
val matrix = [[1, 2], [3, 4], [5, 6]]
say matrix[0]        ~ [1, 2]
say matrix[1][0]     ~ 3
```

---

## 11. Comments

Two styles, both line-based:

```laml
~ This is a tilde comment

# This is a hash comment

say "hello"   ~ inline comment after code
```

Both `~` and `#` cause the rest of the line to be ignored by the lexer. There are no block/multi-line comments.

---

## 12. Built-in Functions

### `say`

```laml
say value              ~ keyword form: prints value + newline, RETURNS the value
say(1, 2, 3)           ~ builtin form: prints all args, returns nil
```

The keyword form returns the printed value, which enables:

```laml
val x = say 42         ~ prints 42, x = 42
```

### `print`

```laml
print "hello"          ~ prints "hello" without newline
print(1, 2, 3)         ~ prints "1 2 3" without newline, returns nil
```

### `type`

Returns the type name as a string:

```laml
type(42)               ~ "int"
type(3.14)             ~ "float"
type("hi")             ~ "string"
type(true)             ~ "bool"
type([1,2])            ~ "array"
type(func(x)x)         ~ "func"
```

### `len`

Returns the length of a string or array:

```laml
len("hello")           ~ 5
len([10, 20, 30])      ~ 3
```

### `sleep`

Sleeps for a given number of milliseconds:

```laml
sleep(1000)            ~ sleep for 1 second
sleep()                ~ sleep for 1000ms (default)
```

### `clock`

Returns the current time in milliseconds since epoch:

```laml
val start = clock()
~ ... do work ...
val elapsed = clock() - start
say "took " + elapsed + "ms"
```

---

## 13. Blocks & Scope

### Blocks

Braces `{ }` create a block. Blocks group statements and create a new scope:

```laml
val a = 1

{
    val b = 2
    say a    ~ 1 (inherited from outer scope)
    say b    ~ 2 (local to block)
}

say b        ~ ERROR: undefined: b
```

### Block Value

A block evaluates to the value of its last expression:

```laml
val result = {
    val x = 10
    val y = 20
    x + y          ~ this is the block's value
}
say result         ~ 30
```

### Empty Block

```laml
val n = {}
say type(n)        ~ "nil" (empty block returns nil)
```

---

## 14. Closures

Functions capture the environment where they are defined:

```laml
func makeAdder(n) {
    func(x) x + n
}

val add5 = makeAdder(5)
val add10 = makeAdder(10)

say add5(3)     ~ 8
say add10(3)    ~ 13
```

Each call to `makeAdder` creates a new closure with its own `n`.

### Counter Pattern

```laml
func makeCounter() {
    val count = 0
    func() {
        count = count + 1
        count
    }
}

val c = makeCounter()
say c()    ~ 1
say c()    ~ 2
say c()    ~ 3
```

This works because `count` is captured by reference (via `shared_ptr<Env>`), and the assignment `count = count + 1` mutates the outer scope's variable.

### Function Factories

```laml
func createComparator(op) {
    if op == "less" {
        func(a, b) a < b
    } else {
        func(a, b) a >= b
    }
}

val less = createComparator("less")
say less(3, 5)    ~ true
say less(5, 3)    ~ false
```

---

## 15. Concurrency with CLOSC

CLOSC (Concurrent Lightweight Ordered Section of Code) lets you run code in its own OS thread.

### Basic CLOSC

```laml
closc background {
    loop true {
        sleep(2000)
        say "background tick"
    }
}

say "main program running"
sleep(5000)
say "main done"
```

The `closc` block starts immediately in a new thread. The main program continues concurrently. When the program exits, all CLOSC sections are joined automatically.

### Named CLOSC Sections

```laml
closc worker1 {
    loop true {
        sleep(1000)
        say "worker 1 tick"
    }
}

closc worker2 {
    loop true {
        sleep(1500)
        say "worker 2 tick"
    }
}
```

The optional name after `closc` is used for debugging.

### CLOSC Lifecycle

- **Start:** A CLOSC section starts an OS thread on declaration.
- **Stop:** On program exit (or `SIGINT`/`SIGTERM`), all sections are stopped gracefully.
- **Clean exit:** Each section's thread is joined before the program terminates.

The running code should check for stop signals — currently there is no forced cancellation. A long-running infinite loop will keep running unless it yields control.

### Use Cases for CLOSC

- Background workers
- Periodic task runners
- Monitoring/dashboard updates
- Parallel computation sections

---

## 16. Error Handling

LAML uses **error-as-values** — there are no exceptions or panics. When an error occurs, it propagates up through the evaluation and terminates the program.

### What Produces Errors

```laml
~ Undefined variable
say unknownVar        ~ Runtime error: undefined: unknownVar

~ Type mismatch in operations
say "hello" - "world" ~ Runtime error: unknown operator: -
  (subtraction is not defined for strings)

~ Division by zero
say 10 / 0            ~ Runtime error: division by zero

~ Integer overflow
say 9223372036854775807 + 1  ~ Runtime error: integer overflow

~ Array bounds
val arr = [1, 2, 3]
say arr[100]          ~ Runtime error: index out of bounds

~ Wrong argument count
func f(a) {}
f(1, 2)               ~ Runtime error: expected 1 args, got 2
```

### Error Propagation

Once an error occurs, it propagates upward through all evaluations. No subsequent code runs:

```laml
say "before"
say 10 / 0            ~ error here
say "after"           ~ never reached
```

### Checking for Errors

There is currently no try/catch mechanism. Errors are fatal. This is by design — LAML prioritizes simplicity over defensive programming. In future versions, error handling constructs may be added.

---

## 17. Intermediate Patterns

### Recursion

```laml
func factorial(n) {
    if n <= 1 {
        1
    } else {
        n * factorial(n - 1)
    }
}

say factorial(5)      ~ 120
```

### Mapping with Functions

```laml
func map(arr, f) {
    val result = []
    loop i in 0 to len(arr) - 1 {
        result = result + [f(arr[i])]
    }
    result
}

say map([1, 2, 3], func(x) x * 10)    ~ [10, 20, 30]
```

### Filtering

```laml
func filter(arr, predicate) {
    val result = []
    loop i in 0 to len(arr) - 1 {
        if predicate(arr[i]) {
            result = result + [arr[i]]
        }
    }
    result
}

say filter([1, 2, 3, 4, 5], func(x) x > 2)    ~ [3, 4, 5]
```

### Reducing

```laml
func reduce(arr, initial, f) {
    val acc = initial
    loop i in 0 to len(arr) - 1 {
        acc = f(acc, arr[i])
    }
    acc
}

val sum = reduce([1, 2, 3, 4, 5], 0, func(a, b) a + b)
say sum    ~ 15
```

### Accumulator / Generator

```laml
func range(start, finish) {
    val result = []
    loop i in start to finish {
        result = result + [i]
    }
    result
}

say range(1, 5)    ~ [1, 2, 3, 4, 5]
```

### Timing Code

```laml
func timeIt(f) {
    val start = clock()
    val result = f()
    val elapsed = clock() - start
    say "took " + elapsed + "ms"
    result
}

timeIt(func() {
    val sum = 0
    loop i in 1 to 100000 {
        sum = sum + i
    }
    sum
})
```

---

## 18. Advanced Topics

### CLOSC + Scheduler Integration

The Scheduler provides a thread-pool-based dataflow execution system. Currently it is initialized on startup (see `main.cpp`) but no LAML-level syntax exposes it yet. Future language versions will allow:

```
~ Future syntax (not yet implemented)
schedule taskA { ... }
schedule taskB { ... } after taskA
schedule taskC { ... } after [taskA, taskB]
```

The underlying C++ infrastructure supports this pattern with dependency tracking, mutex synchronization, and condition-variable-based waiting.

### LLVM JIT Compilation

A JIT engine skeleton exists in `jit.h`/`jit.cpp` using LLVM ORCv2 APIs. Currently:

- `JITEngine::available` returns `false`
- `JITEngine::compile` returns `false`

When activated, it will compile LAML functions to machine code via LLVM IR generation, providing near-native execution speed for hot paths.

### CLOSC Kill Process

For emergency shutdown, CLOSC provides:

```cpp
CloscManager::killProcess()   ~ calls std::_Exit(137)
```

This is triggered by signal handlers (`SIGINT`, `SIGTERM`) in `main.cpp`:

```cpp
void handleSignal(int sig) {
    CloscManager::instance().stopAll();
    Scheduler::instance().stop();
    std::_Exit(sig);
}
```

### Scheduler Thread Pool

The `Scheduler` class manages a thread pool with:

- `std::thread::hardware_concurrency()` workers (minimum 4)
- Mutex + condition variable work queue
- Named tasks with dependency tracking
- `submit()`, `runAndWait()`, `waitFor()`, `markDone()` API

The `Task` struct supports:

```cpp
struct Task {
    std::function<Value()> fn;
    std::string name;
    std::vector<std::string> dependencies;
    // + synchronization primitives
};
```

---

## 19. CLI Reference

```
laml run <file>       Execute a .lm file
laml repl             Start interactive REPL
laml version          Print version string
laml <file>           Shorthand for 'laml run <file>'
```

### REPL Usage

```
$ laml repl
LAML-NG v1.0
Enter code (Ctrl+D to exit):
> say "hello"
hello
> val x = 10 + 20
> say x
30
>
```

### Signal Handling

- `Ctrl+C` (`SIGINT`): Cleanly stops all CLOSC threads and exits
- `SIGTERM`: Same clean shutdown

---

## 20. Language Quirks & Limitations

### val vs let Are Identical

Despite the names, both allow reassignment. This is a known gap — future versions may enforce immutability for `val`.

### No nil Literal

You cannot write `nil` in source. It only appears as:
- Return value of a no-expression `return;`
- Empty block value `{}`
- Some builtin function return values

### No else if

`else if` chains are not supported. Only `if ... else ...` with a single alternative branch.

### No break or continue

Loops have no early exit or skip. Workaround: wrap in a function and use `return`.

### No Object Literals

The `Obj` type exists (property bag wrapping an `Env`) but there's no syntax to create one. Property access via `.` works internally but isn't exposed to user code yet.

### ++ and -- Are Dead

`++` and `--` are lexed but produce `"unknown operator"` at runtime. Use `x = x + 1` instead.

### Semicolons Are Dead

`;` is a valid token but never consumed by the parser. It will likely cause parse errors.

### Range Loops Are Inclusive

`loop i in 1 to 3` gives `1, 2, 3`. The end value is included.

### No String Interpolation

`"Hello, {name}"` does not work. Use concatenation: `"Hello, " + name`.

### CLOSC Thread Safety

CLOSC sections receive a shared `shared_ptr<Env>`. Concurrent reads/writes to variables from multiple threads are **not** protected by mutexes — data races are possible.

---

## Appendix: Quick Reference

### Keywords

```
say     val     let     func    if      else
loop    while   in      to      return  closc
```

### Operators (by precedence)

| Prec | Ops                  | Assoc |
|------|----------------------|-------|
| 8    | `.` `[` `(`          | left  |
| 5    | `*` `/` `%`          | left  |
| 4    | `+` `-`              | left  |
| 3    | `==` `!=` `<` `>` `<=` `>=` | left |
| 2    | `&&` `\|\|`          | left  |
| 1    | `=`                  | right |

### Built-in Functions

| Name    | Returns      |
|---------|-------------|
| `say`   | nil or value |
| `print` | nil         |
| `type`  | string      |
| `len`   | int         |
| `sleep` | nil         |
| `clock` | int         |

### Literal Syntax

| Type    | Example        |
|---------|----------------|
| Int     | `42` `-7`      |
| Float   | `3.14` `-0.5`  |
| String  | `"hello"`      |
| Bool    | `true` `false` |
| Array   | `[1, 2, 3]`    |
| Nil     | (not writable) |

### Sample Program

```laml
~ Fibonacci in LAML
func fib(n) {
    if n <= 1 {
        n
    } else {
        fib(n - 1) + fib(n - 2)
    }
}

loop i in 0 to 10 {
    say "fib(" + i + ") = " + fib(i)
}
```

Output:
```
fib(0) = 0
fib(1) = 1
fib(2) = 1
fib(3) = 2
fib(4) = 3
fib(5) = 5
fib(6) = 8
fib(7) = 13
fib(8) = 21
fib(9) = 34
fib(10) = 55
```
