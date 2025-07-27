# LAML Examples Directory

This directory contains comprehensive examples demonstrating all features of the LAML programming language v3.4.0.

## 🎯 New Examples (Recommended)

### Basic Examples
- **`hello.lm`** - Simple "Hello, World!" program
- **`data_types.lm`** - Complete guide to variables, strings, numbers, booleans, and arrays
- **`interactive_input.lm`** - User input with the `input()` function
- **`format_demo.lm`** - C-style format specifiers and escape sequences

### Intermediate Examples  
- **`control_flow.lm`** - If/else statements, loops, and conditional logic
- **`functions_advanced.lm`** - Function definitions, parameters, return values, and recursion
- **`arrays_advanced.lm`** - Array operations, sorting, searching, and algorithms

### Advanced Examples
- **`objects_advanced.lm`** - Object-oriented programming with properties and methods
- **`tutorial_complete.lm`** - Comprehensive tutorial covering ALL language features

## 🚀 Format Specifiers & Escape Sequences

The new `format_demo.lm` example demonstrates:

### Format Specifiers
- `%s` - String formatting
- `%d` - Integer formatting  
- `%f` - Float formatting (default 6 decimal places)
- `%.2f` - Float with 2 decimal places
- `%c` - Character formatting
- `%b` - Boolean formatting

### Escape Sequences
- `\n` - New line
- `\t` - Tab space
- `\\` - Backslash
- `\'` - Single quote
- `\"` - Double quote
- `\a` - Bell sound
- `\b` - Backspace
- `\r` - Carriage return

### Example Usage
```laml
let name = "Naveen";
let score = 98.7654;
say("Hello %s!\nYour score is %.2f", name, score);
```

## 📚 Legacy Examples

These examples demonstrate specific phases of LAML development:

### Phase 2 Examples
- `phase2_basic.lm` - Basic Phase 2 features
- `phase2_minimal.lm` - Minimal Phase 2 example
- `phase2_simple.lm` - Simple Phase 2 demonstration
- `phase2_types.lm` - Phase 2 type system

### Phase 3 Examples  
- `phase3_comments.lm` - Comment syntax demonstration
- `phase3_complete.lm` - Complete Phase 3 features
- `phase3_demo.lm` - Phase 3 demonstration
- `phase3_errors.lm` - Error handling examples

### Version Examples
- `v3_3_demo.lm` - LAML v3.3.0 features demo
- `v3_3_showcase.lm` - v3.3.0 showcase
- `v3_3_simple.lm` - Simple v3.3.0 example

### Other Examples
- `basic.lm` - Basic language constructs
- `functions.lm` - Simple function examples  
- `objects.lm` - Basic object examples
- `array_operations.lm` - Array operation examples
- `loop_test.lm` - Loop testing
- `range_only.lm` - Range operations
- `type_system.lm` - Type system demonstration
- `lund.lm` - Special example

## 🔧 Comment Syntax

LAML uses unique comment syntax:

```laml
~ This is a single-line comment

{~
   This is a multi-line comment
   that can span multiple lines
~}

let x = 42; ~ Inline comment
```

## 🚀 Running Examples

To run any example:

```bash
# Basic example
./laml run examples/hello.lm

# Interactive example (with input)
echo -e "Alice\n25\nProgramming" | ./laml run examples/interactive_input.lm

# Complete tutorial
./laml run examples/tutorial_complete.lm
```

## 📖 Learning Path

**Recommended learning order:**

1. **Start Here**: `hello.lm` - Get familiar with basic syntax
2. **Data Types**: `data_types.lm` - Learn variables and types  
3. **Control Flow**: `control_flow.lm` - Master if/else and loops
4. **Functions**: `functions_advanced.lm` - Understand functions and scope
5. **Arrays**: `arrays_advanced.lm` - Work with collections
6. **Objects**: `objects_advanced.lm` - Object-oriented programming
7. **Interactive**: `interactive_input.lm` - User input and interaction
8. **Complete**: `tutorial_complete.lm` - Everything together

## 🎯 Key Language Features

- **Variables**: `let`, `val`, `const` declarations
- **Data Types**: integers, floats, strings, booleans, arrays
- **Functions**: Parameters, return values, recursion
- **Objects**: Properties, methods, encapsulation  
- **Control Flow**: `if/else`, `while`, `for` loops
- **Input/Output**: `say` for output, `input()` for user input
- **Comments**: `~` single-line, `{~ ~}` multi-line
- **Arrays**: Declaration, indexing, iteration
- **String Operations**: Concatenation with `+`
- **Boolean Logic**: `&&`, `||`, `!` operators

## 🔍 Syntax Highlights

```laml
~ Import statement
bring xcs.class34;

~ Function definition
func main() {
    ~ Variable declarations
    let name = "LAML";
    val age = 25;
    const VERSION = "3.3.0";
    
    ~ Arrays
    let numbers = [1, 2, 3, 4, 5];
    
    ~ Control flow
    if age >= 18 {
        say "Adult";
    } else {
        say "Minor";
    }
    
    ~ Loops
    while age > 0 {
        say "Age: " + age;
        age--;
    }
    
    ~ User input
    let userName = input("Enter name: ");
    say "Hello, " + userName + "!";
}

~ Object definition
obj Person {
    name = "Unknown";
    age = 0;
    
    func greet() {
        say "Hi, I'm " + name;
    }
}
```

Happy coding with LAML! 🎉
