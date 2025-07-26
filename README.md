# LAML v3.3.0 - Enhanced Data Types, Arrays & Tiger UI Design System

LAML (Low Abstraction Machine Language) is a **sentence-like, general-purpose compiled language** designed for speed, flexibility, and direct kernel interaction. LAML v3.3.0 introduces comprehensive data types, array support, enhanced syntax highlighting, and a beautiful Tiger UI design system.

## 🚀 What's New in v3.3.0

### 🎯 Core Language Enhancements
- **Enhanced Data Types**: Full support for `int`, `float`, `string`, `bool`, and `array` types
- **Advanced Operators**: Comprehensive operator set including arithmetic, comparison, logical, and bitwise operations
- **Array Support**: Complete array functionality with indexing, operations, and type inference
- **Type Inference**: Smart type detection and automatic type promotion

### 🎨 Developer Experience
- **Colored Syntax Highlighting**: Type-aware compiler output with color-coded feedback
- **Tiger UI Design System**: Beautiful Mac OS X Tiger-inspired documentation interface
- **Enhanced Error Messages**: Improved compiler feedback with better debugging information
- **VS Code Integration**: Updated extension with proper installation and syntax support

### 📚 Documentation Improvements
- **Comprehensive Learn Section**: Interactive tutorials covering all LAML features
- **Tiger-Styled Interface**: Authentic Mac OS X Tiger design with Lucida Grande typography
- **Fixed Installation Links**: Direct GitHub release downloads for all platforms
- **Enhanced Examples**: New v3.3.0 demo files showcasing latest features

## 🔧 Installation

### Quick Install (Recommended)

**Windows (PowerShell as Administrator):**
```powershell
iwr -useb https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/windows/install.ps1 | iex
```

**Linux:**
```bash
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/linux/install.sh | bash
```

**macOS:**
```bash
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/macos/install.sh | bash
```

**Termux (Android):**
```bash
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/termux/install.sh | bash
```

### Manual Installation
Download the appropriate binary from [releases](https://github.com/NaveenSingh9999/LAML/releases) and add to your PATH.

### VS Code Extension
The installers will automatically install the VS Code extension if VS Code is detected. You can also download `laml-3.3.0.vsix` from [releases](https://github.com/NaveenSingh9999/LAML/releases) and install manually:
```bash
code --install-extension laml-3.3.0.vsix
```

## 🎯 Quick Start

### Hello World
```laml
bring xcs.class34;

func main() {
    say "Hello, LAML v3.3.0!";
}
```

### Data Types & Arrays
```laml
bring xcs.class34;

func main() {
    ~ Data types with inference
    val name = "LAML";
    val version = 3.3;
    val isStable = true;
    
    ~ Arrays
    val numbers = [10, 20, 30, 40, 50];
    val first = numbers[0];
    
    say "Welcome to " + name + " v" + version;
    say "First number: " + first;
}
```

## 🛠️ Usage

```bash
# Run a LAML program
laml run program.lm

# Compile a LAML program
laml compile program.lm

# Check version
laml version
```

## 📖 Learn More

Visit our [comprehensive documentation](https://naveensingh9999.github.io/LAML/learn.html) with the new Tiger UI design system to explore:

- **Getting Started**: Basic syntax and concepts
- **Data Types**: Complete type system guide
- **Operators**: All available operators and usage
- **Arrays**: Array operations and examples
- **Advanced Features**: Loops, conditionals, and functions

## 🌟 Features

### v3.3.0 Core Features
- ✅ **Enhanced Type System**: Full support for all primary data types
- ✅ **Array Operations**: Complete array functionality with indexing
- ✅ **Advanced Operators**: Arithmetic, comparison, logical, and bitwise
- ✅ **Colored Output**: Type-aware syntax highlighting in compiler
- ✅ **Tiger UI**: Beautiful documentation with authentic design

### Language Features
- ✅ **Sentence-like Syntax**: Natural, readable code structure
- ✅ **Fast Compilation**: Direct compilation to optimized binaries
- ✅ **Cross-platform**: Windows, Linux, and Termux support
- ✅ **VS Code Integration**: Full editor support with syntax highlighting
- ✅ **Zero Dependencies**: Standalone binaries with no runtime requirements

## 🎨 Tiger UI Design System

LAML v3.3.0 introduces the Tiger UI design system, inspired by Mac OS X Tiger's iconic interface:

- **Authentic Typography**: Lucida Grande fonts with proper kerning
- **Classic Color Palette**: Tiger blue (#4A90E2) and refined gradients
- **Window Chrome**: Accurate Tiger-style buttons and window decorations
- **Interactive Elements**: Hover effects and button states matching Tiger

## 📁 Project Structure

```
LAML/
├── cmd/                    # CLI commands and main entry
├── internal/
│   ├── lexer/             # Enhanced lexer with v3.3.0 tokens
│   ├── parser/            # Syntax analysis and AST
│   ├── compiler/          # Code generation
│   └── console/           # Colored output system
├── docs/                  # Tiger UI documentation
├── examples/              # v3.3.0 demo programs
├── vscode-extension/      # VS Code extension v3.3.0
└── installers/           # Platform-specific installers
```

## 🔄 Version History

- **v3.3.0** (2024) - Enhanced Data Types, Arrays & Tiger UI Design System
- **v3.2.0** (2024) - Enhanced User Experience & VS Code Integration  
- **v3.1.0** (2024) - Multi-line Comments & Language Improvements
- **v3.0.0** (2024) - Complete Rewrite with Advanced Features

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Tiger UI Design**: Inspired by Apple's Mac OS X Tiger interface design
- **Go Community**: For the excellent tooling and ecosystem
- **VS Code Team**: For the extensible editor platform
- **LAML Community**: For feedback and contributions to v3.3.0

---

**LAML v3.3.0** - Enhanced Data Types, Arrays & Tiger UI Design System  
*Low Abstraction Machine Language with Advanced Developer Experience*
