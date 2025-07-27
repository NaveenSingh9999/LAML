# LAML v3.4.0 - Format Specifiers, Input Handler And Escape Sequences

LAML (Low Abstraction Machine Language) is a **general-purpose intrepreter/compiler based language** designed for speed, flexibility, and direct kernel interaction. LAML v3.4.0 introduces format specifiers, input handler and escape sequences.

## 🚀 What's New in v3.4.0

### 🎯 Core Language Enhancements
- **C-Style Format Specifiers**: Full support for `%s`, `%d`, `%f`, `%.2f`, `%c`, `%b` format specifiers
- **Escape Sequences**: Complete escape sequence support including `\n`, `\t`, `\\`, `\'`, `\"`, `\a`, `\b`, `\r`
- **Enhanced Say Statement**: New function-style syntax `say("format", args...)` with backward compatibility
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
- **Enhanced Examples**: New v3.4.0 demo files showcasing latest features

## 🔧 Installation

### Quick Install (One-Line Installers)

**Windows (PowerShell as Administrator):**
```powershell
iex ((New-Object System.Net.WebClient).DownloadString('https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1'))
```

**macOS:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/macos/install.sh | bash
```

**Linux (AMD64/x86_64):**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

**Linux (ARM64/aarch64):**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

**Linux (ARMv7):**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

**Termux (Android):**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/termux/install.sh | bash
```

### What the Installers Do

**Windows:**
- ✅ Downloads the Windows executable from GitHub
- ✅ Installs to Program Files with `laml` command
- ✅ Adds to system PATH automatically
- ✅ Creates desktop shortcut and Start Menu entry
- ✅ Installs VS Code extension (if VS Code detected)

**macOS:**
- ✅ Downloads architecture-appropriate binary (Intel/Apple Silicon)
- ✅ Installs to `/usr/local/bin` with `laml` command
- ✅ Creates LAML.app bundle in Applications
- ✅ Sets up shell completion (bash/zsh)
- ✅ Includes automatic architecture detection

**Linux:**
- ✅ Automatic architecture detection (AMD64, ARM64, ARMv7)
- ✅ Installs to `/usr/local/bin` with `laml` command
- ✅ Creates desktop entry for development environment
- ✅ Sets up shell completion (bash/zsh)
- ✅ Multi-distribution support (Ubuntu, Debian, CentOS, Fedora, Arch, openSUSE)

**Termux (Android):**
- ✅ Downloads ARM-optimized binary for Android devices
- ✅ Installs as `laml` command in Termux
- ✅ Creates Termux shortcuts for quick access
- ✅ Verifies installation with comprehensive testing

### Alternative Installation
For manual installation, download the appropriate binary from [Releases](https://github.com/NaveenSingh9999/LAML/releases):
- `laml-windows-x86_64.exe` - Windows 64-bit
- `laml-linux-x86_64` - Linux 64-bit (Intel/AMD)
- `laml-linux-arm64` - Linux ARM64 (Apple Silicon, Raspberry Pi)
- `laml-linux-armv7` - Linux ARMv7 (Raspberry Pi, older ARM)
- `laml-termux` - Android/Termux ARM

### VS Code Extension
The installers automatically detect and install the VS Code extension. You can also download `laml-3.4.0.vsix` from the repository and install manually:
```bash
code --install-extension laml-3.4.0.vsix
```

## 🎯 Quick Start

### Hello World
```laml
bring xcs.class34;

func main() {
    say "Hello, LAML v3.4.0!";
}
```

### Format Specifiers & Data Types
```laml
bring xcs.class34;

func main() {
    ~ Using new format specifiers
    let name = "LAML";
    let version = 3.4;
    let isStable = true;
    
    say("Welcome to %s v%.1f!", name, version);
    say("Status: %b", isStable);
    say("Line 1\nLine 2\tTabbed text");
    
    ~ Arrays
    let numbers = [10, 20, 30, 40, 50];
    let first = numbers[0];
    
    say("First number: %d", first);
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
- **Format Specifiers**: C-style formatting with `%s`, `%d`, `%f`, `%c`, `%b` and escape sequences
- **Data Types**: Complete type system guide
- **Operators**: All available operators and usage
- **Arrays**: Array operations and examples
- **Advanced Features**: Loops, conditionals, and functions

## 🌟 Features

### v3.4.0 Core Features
- ✅ **C-Style Format Specifiers**: Full printf-style formatting support
- ✅ **Escape Sequences**: Complete escape sequence processing
- ✅ **Enhanced Say Statement**: Function-style syntax with backward compatibility
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

LAML v3.4.0 introduces the Tiger UI design system, inspired by Mac OS X Tiger's iconic interface:

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

- **v3.4.0** (2025) - Format Specifiers, Input Handler And Escape Sequences
- **v3.3.0** (2025) - Enhanced User Experience & VS Code Integration  
- **v3.2.0** (2025) - Multi-line Comments & Language Improvements
- **v3.1.0** (2025) - Complete Rewrite with Advanced Features

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---