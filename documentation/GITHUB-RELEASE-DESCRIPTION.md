# LAML v3.3.0 Installer Fix Release

## 🚨 Critical Architecture Compatibility Update

This release addresses a critical issue where Termux users on Android devices encountered "Exec format error" due to architecture incompatibility. The installers have been completely overhauled with automatic architecture detection and platform-specific binary selection.

## 🔧 What's Fixed

### Architecture Compatibility
- **Termux/Android**: Now uses ARM-optimized `laml-termux` binary instead of x86_64
- **Linux**: Automatic detection for x86_64, ARM64, and ARMv7 architectures  
- **macOS**: Support for both Intel (x86_64) and Apple Silicon (ARM64)
- **Windows**: Improved PowerShell installer with better error handling

### Installer Improvements
- ✅ All URLs updated to use `raw.githubusercontent.com` format
- ✅ Added architecture detection functions
- ✅ Better error messages and troubleshooting guidance
- ✅ Improved file verification and download validation
- ✅ Cross-platform compatibility testing

### Documentation Updates
- ✅ Updated setup instructions with architecture information
- ✅ Fixed copy button functionality across all documentation
- ✅ Added troubleshooting section for common issues
- ✅ Comprehensive installation verification steps

## 📦 Release Assets

- `laml-v3.3.0-updated.tar.gz` - Complete release package with all binaries and documentation
- `laml-v3.3.0-updated.zip` - Windows-compatible archive format  

## 🛠️ Supported Architectures

| Platform | Architecture | Binary |
|----------|-------------|--------|
| Linux | x86_64 | `laml-linux-x86_64` |
| Linux | ARM64 | `laml-linux-arm64` |
| Linux | ARMv7 | `laml-linux-armv7` |
| macOS | Intel | `laml-linux-x86_64` |
| macOS | Apple Silicon | `laml-linux-arm64` |
| Android/Termux | ARM | `laml-termux` |
| Windows | x86_64 | `laml-windows-x86_64.exe` |

## 🚀 Installation

### One-Line Installers (Recommended)

**Linux:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

**macOS:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/macos/install.sh | bash
```

**Termux (Android):**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/termux/install.sh | bash
```

**Windows (PowerShell as Administrator):**
```powershell
iex ((New-Object System.Net.WebClient).DownloadString('https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1'))
```

## ✅ Verification

After installation:
```bash
laml version
echo 'bring xcs.class34; func main() { say "Hello, LAML v3.3.0!"; }' > hello.lm
laml run hello.lm
```

## 🐛 Bug Reports

If you encounter "Exec format error" or other architecture-related issues:
1. Check your architecture: `uname -m`
2. Verify the correct binary is downloaded
3. Report issues with system details at: https://github.com/NaveenSingh9999/LAML/issues

---

**Full Changelog:** [CHANGELOG-v3.3.0.md](./CHANGELOG-v3.3.0.md)  
**Installation Guide:** [INSTALL.md](./INSTALL.md)  
**Release Notes:** [RELEASE-NOTES-v3.3.0-updated.md](./RELEASE-NOTES-v3.3.0-updated.md)
