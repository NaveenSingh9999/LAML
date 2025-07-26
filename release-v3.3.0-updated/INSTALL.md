# LAML Quick Install Scripts

## Windows (PowerShell)
```powershell
# Run as Administrator
iwr -useb https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/windows/install.ps1 | iex
```

## Linux
```bash
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/linux/install.sh | bash
```

## macOS
```bash
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/macos/install.sh | bash
```

## Termux (Android)
```bash
curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/installers/termux/install.sh | bash
```

## Uninstall
Each installer supports uninstall:
- Windows: `.\install.ps1 -Uninstall`
- Linux/macOS/Termux: `./install.sh --uninstall`

## Features
- ✅ Downloads universal `laml` binary from GitHub
- ✅ Installs as `laml` command (not platform-specific names)
- ✅ Works from anywhere (added to PATH)
- ✅ Automatic dependency checking
- ✅ Shell completion support
- ✅ VS Code extension installation (when available)
- ✅ Clean uninstall option
- ✅ No platform-specific binaries needed in package
