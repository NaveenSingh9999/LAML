#!/bin/bash

# LAML Master Build Script
# Creates all installers and packages for distribution

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    print_colored $CYAN "🏗️  LAML Universal Build System"
    print_colored $CYAN "==============================="
}

detect_os() {
    case "$(uname -s)" in
        Linux*)     machine=Linux;;
        Darwin*)    machine=Mac;;
        CYGWIN*)    machine=Cygwin;;
        MINGW*)     machine=MinGw;;
        *)          machine="UNKNOWN:$(uname -s)"
    esac
    echo $machine
}

build_compiler() {
    print_colored $YELLOW "🔨 Building LAML compiler..."
    
    # Ensure we have a clean build
    rm -f laml laml-*
    
    # Build for current platform
    print_colored $BLUE "Building for current platform..."
    go build -ldflags="-s -w" -o laml
    
    # Cross-compile for other platforms
    print_colored $BLUE "Cross-compiling for multiple platforms..."
    
    # Windows (x86_64)
    GOOS=windows GOARCH=amd64 go build -ldflags="-s -w" -o laml-windows-x86_64.exe
    
    # Linux (x86_64)
    GOOS=linux GOARCH=amd64 go build -ldflags="-s -w" -o laml-linux-x86_64
    
    # Linux (ARM64)
    GOOS=linux GOARCH=arm64 go build -ldflags="-s -w" -o laml-linux-arm64
    
    # Linux (ARMv7 for Termux)
    GOOS=linux GOARCH=arm GOARM=7 go build -ldflags="-s -w" -o laml-linux-armv7
    
    # Verify binaries were created
    if [ ! -f "laml" ] || [ ! -f "laml-windows-x86_64.exe" ] || [ ! -f "laml-linux-x86_64" ]; then
        print_colored $RED "❌ Failed to build all binaries"
        exit 1
    fi
    
    print_colored $GREEN "✅ Compiler built for all platforms"
}

create_release_directory() {
    print_colored $YELLOW "📁 Creating release directory..."
    
    rm -rf release
    mkdir -p "release/laml-v3.2.0"/{windows,linux,termux,vscode-extension}
    
    print_colored $GREEN "✅ Release directory created"
}

package_windows() {
    local release_dir=$1
    print_colored $YELLOW "📦 Packaging Windows installer..."
    
    # Copy REAL Windows binary
    if [ ! -f "laml-windows-x86_64.exe" ]; then
        print_colored $RED "❌ Windows binary not found!"
        exit 1
    fi
    
    cp laml-windows-x86_64.exe "$release_dir/windows/laml.exe"
    
    # Copy installer script
    cp installers/windows/install.ps1 "$release_dir/windows/"
    
    # Create batch file wrapper
    cat > "$release_dir/windows/install.bat" << 'EOF'
@echo off
echo 🚀 LAML Windows Installer
echo ========================
echo This will install LAML on your Windows system.
echo.
echo Administrator privileges required.
echo.
pause
powershell -ExecutionPolicy Bypass -File "%~dp0install.ps1"
pause
EOF
    
    # Create simple README
    cat > "$release_dir/windows/README.txt" << 'EOF'
LAML for Windows
================

Installation:
1. Right-click "install.bat" and "Run as administrator"
2. Follow the installation prompts
3. Restart your command prompt
4. Type "laml version" to verify

Manual Installation:
1. Run PowerShell as Administrator
2. Execute: .\install.ps1

Uninstallation:
Run PowerShell as Administrator and execute:
.\install.ps1 -Uninstall

For support, visit: https://github.com/NaveenSingh9999/LAML
EOF
    
    print_colored $GREEN "✅ Windows package ready"
}

package_linux() {
    local release_dir=$1
    print_colored $YELLOW "📦 Packaging Linux installer..."
    
    # Verify Linux binaries exist
    if [ ! -f "laml-linux-x86_64" ] || [ ! -f "laml-linux-arm64" ]; then
        print_colored $RED "❌ Linux binaries not found!"
        exit 1
    fi
    
    # Copy REAL Linux binaries
    cp laml-linux-x86_64 "$release_dir/linux/laml-x86_64"
    cp laml-linux-arm64 "$release_dir/linux/laml-arm64"
    
    # Make binaries executable
    chmod +x "$release_dir/linux/laml-x86_64"
    chmod +x "$release_dir/linux/laml-arm64"
    
    # Copy installer script
    cp installers/linux/install.sh "$release_dir/linux/"
    chmod +x "$release_dir/linux/install.sh"
    
    # Create simple installer wrapper
    cat > "$release_dir/linux/install-laml.sh" << 'EOF'
#!/bin/bash
echo "🚀 LAML Linux Installer"
echo "======================="
echo "This will install LAML on your Linux system."
echo ""
echo "Detected architecture: $(uname -m)"
echo ""
read -p "Continue? (y/N): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    bash "$(dirname "$0")/install.sh"
else
    echo "Installation cancelled."
fi
EOF
    
    chmod +x "$release_dir/linux/install-laml.sh"
    
    # Create README
    cat > "$release_dir/linux/README.md" << 'EOF'
# LAML for Linux

## Quick Installation

```bash
curl -sSL https://github.com/NaveenSingh9999/LAML/releases/latest/download/install-laml.sh | bash
```

## Manual Installation

1. Make the installer executable:
   ```bash
   chmod +x install-laml.sh
   ```

2. Run the installer:
   ```bash
   ./install-laml.sh
   ```

## Supported Architectures

- x86_64 (Intel/AMD 64-bit)
- ARM64 (ARM 64-bit)
- ARMv7 (ARM 32-bit)

## Uninstallation

```bash
./install.sh --uninstall
```

## Requirements

- Linux kernel 3.10+
- curl and unzip (installed automatically)
- sudo access for system installation

For support, visit: https://github.com/NaveenSingh9999/LAML
EOF
    
    print_colored $GREEN "✅ Linux package ready"
}

package_termux() {
    local release_dir=$1
    print_colored $YELLOW "📦 Packaging Termux installer..."
    
    # Verify Termux binary exists
    if [ ! -f "laml-linux-armv7" ]; then
        print_colored $RED "❌ Termux binary not found!"
        exit 1
    fi
    
    # Copy Termux binary with the correct name that installer expects
    cp laml-linux-armv7 "$release_dir/termux/laml"
    chmod +x "$release_dir/termux/laml"
    
    # Copy installer script
    cp installers/termux/install.sh "$release_dir/termux/"
    chmod +x "$release_dir/termux/install.sh"
    
    # Create one-liner installer
    cat > "$release_dir/termux/install-termux.sh" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo "🚀 LAML Termux Installer"
echo "========================"
echo "Installing LAML for Android/Termux..."
echo ""

# Download and run installer
if command -v curl >/dev/null 2>&1; then
    bash "$(dirname "$0")/install.sh"
else
    echo "Installing curl first..."
    pkg install -y curl
    bash "$(dirname "$0")/install.sh"
fi
EOF
    
    chmod +x "$release_dir/termux/install-termux.sh"
    
    # Create README
    cat > "$release_dir/termux/README.md" << 'EOF'
# LAML for Termux (Android)

## Installation

1. Open Termux
2. Install dependencies:
   ```bash
   pkg update && pkg install curl unzip
   ```

3. Download and run installer:
   ```bash
   curl -sSL https://github.com/NaveenSingh9999/LAML/releases/latest/download/install-termux.sh | bash
   ```

## Manual Installation

1. Make the installer executable:
   ```bash
   chmod +x install-termux.sh
   ```

2. Run the installer:
   ```bash
   ./install-termux.sh
   ```

## Features

- Native ARM/ARM64 compilation
- Android storage integration
- Termux widget shortcuts
- Development environment setup

## Requirements

- Termux app from F-Droid or Google Play
- Android 5.0+ (API level 21+)
- Storage permission (granted during installation)

## Quick Start

After installation:
```bash
cd ~/laml-examples
laml run hello-termux.lm
```

For support, visit: https://github.com/NaveenSingh9999/LAML
EOF
    
    print_colored $GREEN "✅ Termux package ready"
}

build_vscode_extension() {
    local release_dir=$1
    print_colored $YELLOW "📝 Building VS Code extension..."
    
    # Check if we can build VSIX
    if command -v npm >/dev/null 2>&1; then
        print_colored $BLUE "Building VSIX package..."
        
        cd vscode-extension
        
        # Install vsce if not available
        if ! command -v vsce >/dev/null 2>&1; then
            npm install -g vsce
        fi
        
        # Copy icon (placeholder - in real scenario, convert SVG to PNG)
        cp icons/laml-icon.svg icons/laml-icon.png
        
        # Package extension
        vsce package --no-yarn --out "../$release_dir/vscode-extension/laml-3.2.0.vsix"
        
        cd ..
        
        print_colored $GREEN "✅ VS Code extension packaged"
    else
        print_colored $YELLOW "⚠️  npm not found, copying extension files..."
        cp -r vscode-extension/* "$release_dir/vscode-extension/"
    fi
    
    # Create installation instructions
    cat > "$release_dir/vscode-extension/README.md" << 'EOF'
# LAML VS Code Extension

## Installation

### Method 1: VSIX Package (Recommended)
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Click the "..." menu and select "Install from VSIX..."
4. Select the `laml-3.2.0.vsix` file

### Method 2: Manual Installation
1. Copy the extension folder to your VS Code extensions directory:
   - Windows: `%USERPROFILE%\.vscode\extensions\`
   - macOS: `~/.vscode/extensions/`
   - Linux: `~/.vscode/extensions/`

## Features

- Syntax highlighting for .lm files
- Comment support (~ and {~ ~})
- Auto-completion
- File icons
- Language configuration

## File Association

The extension automatically associates `.lm` files with LAML syntax highlighting.

## Development

To contribute to the extension:
1. Install Node.js and npm
2. Install vsce: `npm install -g vsce`
3. Make changes to the extension
4. Package: `vsce package`

For support, visit: https://github.com/NaveenSingh9999/LAML
EOF
    
    print_colored $GREEN "✅ VS Code extension ready"
}

create_universal_installer() {
    local release_dir=$1
    print_colored $YELLOW "🌍 Creating universal installer..."
    
    cat > "$release_dir/install.sh" << 'EOF'
#!/bin/bash

# LAML Universal Installer
# Detects platform and runs appropriate installer

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_colored() {
    echo -e "${1}${2}${NC}"
}

detect_platform() {
    if [ -d "/data/data/com.termux" ]; then
        echo "termux"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

main() {
    print_colored $CYAN "🚀 LAML Universal Installer"
    print_colored $CYAN "=========================="
    
    local platform=$(detect_platform)
    print_colored $BLUE "📱 Detected platform: $platform"
    
    case $platform in
        "termux")
            print_colored $YELLOW "🤖 Running Termux installer..."
            if [ -f "termux/install.sh" ]; then
                bash termux/install.sh
            else
                print_colored $RED "❌ Termux installer not found!"
                exit 1
            fi
            ;;
        "linux")
            print_colored $YELLOW "🐧 Running Linux installer..."
            if [ -f "linux/install.sh" ]; then
                bash linux/install.sh
            else
                print_colored $RED "❌ Linux installer not found!"
                exit 1
            fi
            ;;
        "windows")
            print_colored $YELLOW "🪟 Running Windows installer..."
            print_colored $BLUE "Please run windows/install.bat as Administrator"
            ;;
        "macos")
            print_colored $YELLOW "🍎 macOS support coming soon!"
            print_colored $BLUE "For now, use the Linux installer"
            ;;
        *)
            print_colored $RED "❌ Unsupported platform: $platform"
            print_colored $YELLOW "Please install manually or contact support"
            exit 1
            ;;
    esac
}

main "$@"
EOF
    
    chmod +x "$release_dir/install.sh"
    
    print_colored $GREEN "✅ Universal installer created"
}

create_documentation() {
    local release_dir=$1
    print_colored $YELLOW "📚 Creating documentation..."
    
    cat > "$release_dir/README.md" << 'EOF'
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
EOF
    
    print_colored $GREEN "✅ Documentation created"
}

main() {
    print_header
    
    print_colored $BLUE "🖥️  Current OS: $(detect_os)"
    print_colored $BLUE "📁 Working directory: $(pwd)"
    
    # Build compiler for all platforms
    build_compiler
    
    # Create release structure
    create_release_directory
    local release_dir="release/laml-v3.2.0"
    print_colored $GREEN "📁 Using release directory: $release_dir"
    
    # Package for each platform
    package_windows "$release_dir"
    package_linux "$release_dir"
    package_termux "$release_dir"
    build_vscode_extension "$release_dir"
    
    # Create universal installer and docs
    create_universal_installer "$release_dir"
    create_documentation "$release_dir"
    
    # Create archive
    print_colored $YELLOW "📦 Creating release archive..."
    cd release
    tar -czf "laml-v3.2.0.tar.gz" "laml-v3.2.0/"
    zip -r "laml-v3.2.0.zip" "laml-v3.2.0/"
    cd ..
    
    echo ""
    print_colored $GREEN "🎉 LAML v3.2.0 build completed successfully!"
    echo ""
    print_colored $CYAN "📦 Release packages:"
    print_colored $NC "• $release_dir/ - Complete installer package"
    print_colored $NC "• release/laml-v3.2.0.tar.gz - Linux/macOS archive"
    print_colored $NC "• release/laml-v3.2.0.zip - Windows archive"
    echo ""
    print_colored $CYAN "🚀 Ready for distribution!"
    print_colored $BLUE "Upload to GitHub Releases or distribute directly"
}

# Run main function
main "$@"
