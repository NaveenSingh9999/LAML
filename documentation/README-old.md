# 🌐 LAML - Low Abstraction Machine Language

## 🔰 Introduction

LAML is a low-abstraction, sentence-like general-purpose compiled language, built for speed, flexibility, and direct kernel interaction in mind. It uses a `.lm` extension, and compiles through a Golang-powered ultra-light compiler.

This language is meant to blend human-readability with machine control, allowing developers to:

- Write programs using highly readable commands,
- Skip complex syntax,
- Yet access powerful features like OOP, memory control, parallel execution, and system-level access.

LAML aims to be:
- **Minimal** (lightweight footprint),
- **Fast** (compiled with Golang backend),
- **Expressive** (easy-to-learn syntax),
- **Secure & Scalable** (inspired by system-level lang design like Zig, Go, and C).

---

## ⚙️ Phase 1: Core Architecture Design

---

### 🧱 1. Compiler Core

| Component | Description |
|-----------|-------------|
| **Lexer** | Breaks down `.lm` code into tokens (keywords, operators, identifiers). |
| **Parser** | Validates syntax structure and creates an Abstract Syntax Tree (AST). |
| **AST Engine** | AST is analyzed for logic, flow, and code blocks. |
| **CodeGen** | Converts AST to Go-equivalent logic or binary-intermediate. |
| **Optimizer** | Shrinks and simplifies before compilation. |
| **Binary Output** | Produces a `.bin` file (or ELF/executable) for system use. |

---

### 📦 2. Import System

Every `.lm` file starts with mandatory imports to access class-like modules:

```laml
bring xcs.class34;
bring xcn.classconstant5;
bring xca.patentu3;
```

These are abstracted core libraries:
- **xcs.class34**: Core system operations (print, IO, sleep, threads).
- **xcn.classconstant5**: Constants and low-level memory.
- **xca.patentu3**: Custom logic, crypto, security primitives.

✔️ Each module can be versioned (class34) and will be mapped inside a local LAML Library cache in `/laml_modules/`.

---

### 🔤 3. Variables & Constants

```laml
let user = "lamgerr";
const max_limit = 100;
```

- `let` declares mutable variables.
- `const` creates fixed values.

✔️ Ends with semicolon. ✔️ Supports strings, numbers, bools, arrays.

---

### 🧮 4. Functions

```laml
func greet(name) {
    say "Hello, " + name;
}
```

- `func` defines a block.
- `say` is LAML's built-in print function (wrapped in Go fmt).

Return values:

```laml
func add(x, y) {
    return x + y;
}
```

---

### 🔁 5. Conditionals and Loops

```laml
if user = "lamgerr" {
    say "Access granted";
} else {
    say "Invalid user";
}
```

Loop example:

```laml
loop x in 0 to 10 {
    say x;
}
```

While loop:

```laml
while i < 5 {
    say i;
    i = i + 1;
}
```

---

### 🧊 6. Objects / OOP

```laml
obj Car {
    brand = "Tesla";
    speed = 0;

    func drive(km) {
        speed = speed + km;
    }
}
```

Create instance:

```laml
let myCar = Car();
myCar.drive(20);
say myCar.speed;
```

---

### 🛠️ 7. File System Access

```laml
bring xfs.fileio7;

func read_file(path) {
    let content = file.read(path);
    say content;
}
```

You'll use `file.read`, `file.write`, `file.delete`, etc., from xfs.

---

### 🧩 Sample Hello World in LAML:

```laml
bring xcs.class34;

func main() {
    say "Yo World!";
}
```

---

### 🧠 Closing Notes (Phase 1)

✅ Defined imports and modular system.  
✅ Syntax = simplified but powerful.  
✅ Base compiler flow ready.  
✅ OOP & core features included.  
✅ Next: Building compiler in Go, setting up .lm parser.