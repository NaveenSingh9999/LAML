# LAML v3.3.0 (Updated) - Fixed Installation URLs

🛠️ **This is an updated release focusing on installer fixes and documentation improvements.**

## 🔧 Installation Fixes

### Fixed GitHub Raw URLs
- ✅ Updated all installers to use `raw.githubusercontent.com` instead of `github.com/raw/`
- ✅ Fixed binary download URLs across all platforms
- ✅ Updated documentation with correct installation commands

### Architecture-Aware Installation
- ✅ **Fixed Termux ARM compatibility** - Now uses `laml-termux` binary for Android devices
- ✅ **Linux architecture detection** - Automatically downloads the correct binary:
  - x86_64: `laml-linux-x86_64`
  - ARM64: `laml-linux-arm64` 
  - ARMv7: `laml-linux-armv7`
- ✅ **macOS architecture detection** - Supports both Intel and Apple Silicon
- ✅ **Improved error handling** - Better temp file management and verification

### Updated Installation Methods

**Linux:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

**macOS:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/macos/install.sh | bash
```

**Windows (PowerShell as Admin):**
```powershell
iwr -useb https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1 | iex
```

**Termux (Android):**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/termux/install.sh | bash
```

## 📚 Documentation Improvements

### Enhanced Learn Page
- ✅ Fixed CSS styling and layout issues
- ✅ Removed duplicate content in learn.css
- ✅ Improved responsive design for mobile devices
- ✅ Added proper syntax highlighting for LAML code examples

### Updated Setup Documentation
- ✅ Added alternative installation methods with `chmod +x` commands
- ✅ Fixed copy button functionality across all installation cards
- ✅ Updated verification commands (`laml version` instead of `laml --version`)
- ✅ Added manual download and installation instructions

## 🚀 Core Features (v3.3.0)

LAML v3.3.0 continues to provide:

### Enhanced Data Types
- **Integers**: `val age = 25;`
- **Floats**: `val pi = 3.14159;`
- **Strings**: `val name = "LAML";`
- **Booleans**: `val isReady = true;`
- **Arrays**: `val numbers = [1, 2, 3, 4, 5];`

### Core Language Features
- Function definitions with `func`
- Module imports with `bring xcs.class34;`
- Output with `say "text";`
- Comments with `~` prefix
- Array operations and indexing

### Developer Experience
- VS Code extension with syntax highlighting
- Cross-platform installers (Linux, macOS, Windows, Termux)
- Shell completion (bash, zsh)
- Comprehensive documentation and tutorials

## 🛠️ Technical Changes

### Installer Updates
- All platform installers now download universal `laml` binary from GitHub
- Consistent installation to `/usr/local/bin/laml` on Unix systems
- Windows installer uses `%LOCALAPPDATA%\LAML\` directory
- Fixed PATH management across all platforms

### Binary Distribution
- Universal binary works across all supported platforms
- Downloads from: `https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml`
- No more platform-specific binaries needed

## 📋 Installation Verification

After installation, verify LAML is working:

```bash
# Check version
laml version

# Create and run hello world
echo 'bring xcs.class34; func main() { say "Hello, LAML v3.3.0!"; }' > hello.lm
laml run hello.lm
```

## 🛠️ Troubleshooting

### "cannot execute binary file: Exec format error"
This error means the binary architecture doesn't match your system. The updated installers now automatically detect and download the correct architecture:

- **Linux**: Supports x86_64, ARM64, ARMv7
- **macOS**: Supports Intel (x86_64) and Apple Silicon (ARM64)  
- **Termux**: Uses ARM-optimized binary for Android devices

### Download Issues
If installation fails:
1. Check internet connection
2. Verify you can access GitHub: `curl -I https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-termux`
3. Try manual installation from the alternative methods section

## 🔗 Quick Links

- **Documentation**: [Learn LAML](https://naveensingh9999.github.io/LAML/learn.html)
- **Installation Guide**: [Setup Instructions](https://naveensingh9999.github.io/LAML/setup.html)
- **Feature Overview**: [Features](https://naveensingh9999.github.io/LAML/features.html)
- **FAQ**: [Frequently Asked Questions](https://naveensingh9999.github.io/LAML/faq.html)

## 🐛 Bug Reports

If you encounter any issues with installation or usage, please report them at:
https://github.com/NaveenSingh9999/LAML/issues

---

**Note**: This is an updated release focusing on installer reliability and documentation improvements. The core LAML language features remain the same as v3.3.0.
