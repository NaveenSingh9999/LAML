# LAML v3.4.0 - C-Style Format Specifiers and Escape Sequences

## 🎯 Major Features

### 🆕 C-Style Format Specifiers
- **%s** - String formatting
- **%d** - Integer formatting  
- **%f** - Float formatting (default precision)
- **%.2f** - Float formatting with precision control
- **%c** - Character formatting
- **%b** - Boolean formatting

### 🔤 Escape Sequences Support
- **\n** - Newline
- **\t** - Tab character
- **\\** - Backslash
- **\'** - Single quote
- **\"** - Double quote
- **\a** - Bell/alert
- **\b** - Backspace
- **\r** - Carriage return

### 🔧 Enhanced Say Statement
- New function-style syntax: `say("format", args...)`
- Full backward compatibility with old syntax: `say "string"`
- Automatic type conversion and validation
- Comprehensive error handling

## 📝 Code Examples

### Basic Format Specifiers
```laml
let name = "LAML";
let version = 3.4;
let isStable = true;

say("Welcome to %s v%.1f!", name, version);
say("Status: %b", isStable);
```

### Escape Sequences
```laml
say("Line 1\nLine 2\tTabbed text");
say("Quote: \"Hello World\"");
say("Path: \\home\\user\\file");
```

### Combined Usage
```laml
let player = "Lamgerr";
let score = 97.45;
say("Player: %s\nScore: %.2f\nResult: %s", player, score, "PASS");
```

## 🛠️ Technical Improvements

### Parser Enhancements
- Enhanced AST with Format and Arguments fields in SayStatement
- Function-style argument parsing for new say syntax
- Maintained backward compatibility with existing code

### Evaluator Updates
- New format processing engine with regex-based parsing
- Automatic type conversion system
- Comprehensive error handling and validation
- Support for mixed argument types

### Documentation
- Updated learn.html with comprehensive format specifiers tutorial
- Added interactive tables and examples
- Enhanced Tiger UI styling for better presentation
- Complete format_demo.lm example file

## 📦 Installation

All platform binaries have been updated to v3.4.0:
- `laml-windows-x86_64.exe` - Windows 64-bit
- `laml-linux-x86_64` - Linux 64-bit (Intel/AMD)
- `laml-linux-arm64` - Linux ARM64 (Apple Silicon, Raspberry Pi)
- `laml-linux-armv7` - Linux ARMv7 (Raspberry Pi, older ARM)

### Quick Install
Use our one-line installers for automatic setup:

**Windows:**
```powershell
iex ((New-Object System.Net.WebClient).DownloadString('https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1'))
```

**macOS/Linux:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

## 🔄 Migration Guide

Existing LAML code continues to work without changes. To use new features:

### Old Syntax (Still Works)
```laml
say "Hello World";
say "Line 1\nLine 2";
```

### New Syntax (Recommended)
```laml
say("Hello %s", "World");
say("Line 1\nLine 2");
```

## 🐛 Bug Fixes
- Fixed format string parsing edge cases
- Improved type conversion error handling
- Enhanced escape sequence processing
- Better error messages for invalid format specifiers

## 📚 Documentation Updates
- Comprehensive format specifiers tutorial in learn.html
- Enhanced Tiger UI design with better tables
- Updated README with v3.4.0 examples
- New format_demo.lm showcase file

## 🚀 What's Next
LAML v3.4.0 brings powerful formatting capabilities while maintaining simplicity and performance. Future releases will continue to enhance the developer experience with more language features and tooling improvements.

---

**Full Changelog**: https://github.com/NaveenSingh9999/LAML/compare/v3.3.0...v3.4.0
