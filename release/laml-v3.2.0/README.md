# LAML v3.2.0 - Release Package

Welcome to LAML (Low Abstraction Machine Language) - a modern compiled language with enhanced developer experience.

## 🚀 Quick Installation

### Universal Installer (Recommended)
```bash
bash install.sh
```

### Platform-Specific Installation

#### Windows
1. Run `windows/install.bat` as Administrator
2. Follow the installation wizard

#### Linux
```bash
bash linux/install.sh
```

#### Termux (Android)
```bash
bash termux/install.sh
```

#### VS Code Extension
- Install `vscode-extension/laml-3.2.0.vsix` via VS Code

## 📦 Package Contents

```
laml-v3.2.0/
├── install.sh              # Universal installer
├── windows/                 # Windows installer and binaries
├── linux/                   # Linux installer and binaries
├── termux/                  # Termux installer and binaries
├── vscode-extension/        # VS Code extension package
└── README.md               # This file
```

## 🎯 What's New in v3.2.0

### Enhanced User Experience
- ✅ Clean compiler output (no decorative headers)
- ✅ VS Code run button for .lm files  
- ✅ Simplified terminal execution
- ✅ One-click file execution from editor
- ✅ Improved error messages

### Previous Features (v3.0.0)
- ✅ Comments system (~ and {~ ~})
- ✅ Styled console output with emojis
- ✅ VS Code syntax highlighting
- ✅ Cross-platform installers
- ✅ File icon integration

### Language Features
- Modern syntax with intuitive keywords
- val/let/const variable declarations
- Function definitions and calls
- Control flow (if/else, loops)
- String and numeric operations
- Comments support

## 🔧 System Requirements

### Windows
- Windows 10 or later
- PowerShell 5.0+
- Administrator privileges

### Linux
- Linux kernel 3.10+
- curl and unzip
- sudo access

### Termux
- Android 5.0+ (API 21+)
- Termux app installed
- Storage permission

## 📖 Quick Start

After installation:

1. Verify installation:
   ```bash
   laml version
   ```

2. Create your first LAML file (`hello.lm`):
   ```laml
   ~ Hello World in LAML
   bring xcs.class34;
   
   func main() {
       say "Hello, LAML World!";
   }
   ```

3. Run your program:
   ```bash
   laml run hello.lm
   ```

## 📚 Documentation

- GitHub: https://github.com/NaveenSingh9999/LAML
- Issues: https://github.com/NaveenSingh9999/LAML/issues
- Examples: Check the examples/ directory after installation

## 🆘 Support

If you encounter any issues:
1. Check the platform-specific README files
2. Visit our GitHub issues page
3. Join our community discussions

---

**LAML v3.2.0 - Where developer experience meets machine performance!** 🎉
