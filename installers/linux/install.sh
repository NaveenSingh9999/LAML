#!/bin/bash

# LAML Installer for Linux
# Downloads and installs LAML from GitHub with architecture detection

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
INSTALL_DIR="/usr/local/bin"
CONFIG_DIR="$HOME/.config/laml"
DESKTOP_DIR="$HOME/.local/share/applications"

# GitHub URLs with architecture detection
detect_architecture() {
    local arch=$(uname -m)
    case $arch in
        x86_64|amd64)
            LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-linux-x86_64"
            ARCH_NAME="x86_64 (AMD64/Intel 64-bit)"
            ;;
        aarch64|arm64)
            LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-linux-arm64"
            ARCH_NAME="ARM64 (64-bit ARM)"
            ;;
        armv7l|armhf|armv7)
            LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-linux-armv7"
            ARCH_NAME="ARMv7 (32-bit ARM)"
            ;;
        *)
            print_colored $RED "❌ Unsupported architecture: $arch"
            print_colored $YELLOW "Supported architectures:"
            print_colored $YELLOW "  • x86_64/amd64 (Intel/AMD 64-bit)"
            print_colored $YELLOW "  • aarch64/arm64 (64-bit ARM)"
            print_colored $YELLOW "  • armv7l/armhf (32-bit ARM)"
            exit 1
            ;;
    esac
}

print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    print_colored $CYAN "🚀 LAML Installer for Linux"
    print_colored $CYAN "============================"
    print_colored $BLUE "� Multi-architecture support (x86_64, ARM64, ARMv7)"
    echo
}

check_linux() {
    if [[ "$OSTYPE" != "linux-gnu"* ]]; then
        print_colored $RED "❌ This installer is designed for Linux only!"
        print_colored $YELLOW "Please use the appropriate installer for your platform."
        exit 1
    fi
    
    print_colored $GREEN "✅ Linux environment detected"
    
    # Detect Linux distribution for better help messages
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$NAME
        print_colored $BLUE "📋 Distribution: $DISTRO"
    fi
}

check_dependencies() {
    print_colored $YELLOW "🔍 Checking dependencies..."
    
    # Check for required tools
    local missing_deps=()
    
    if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
        missing_deps+=("curl or wget")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_colored $RED "❌ Missing required dependencies:"
        for dep in "${missing_deps[@]}"; do
            print_colored $RED "   - $dep"
        done
        echo
        print_colored $YELLOW "Please install the missing dependencies and try again."
        
        # Distribution-specific installation commands
        if command -v apt >/dev/null 2>&1; then
            print_colored $YELLOW "On Ubuntu/Debian: sudo apt update && sudo apt install curl"
        elif command -v yum >/dev/null 2>&1; then
            print_colored $YELLOW "On CentOS/RHEL: sudo yum install curl"
        elif command -v dnf >/dev/null 2>&1; then
            print_colored $YELLOW "On Fedora: sudo dnf install curl"
        elif command -v pacman >/dev/null 2>&1; then
            print_colored $YELLOW "On Arch: sudo pacman -S curl"
        elif command -v zypper >/dev/null 2>&1; then
            print_colored $YELLOW "On openSUSE: sudo zypper install curl"
        else
            print_colored $YELLOW "Install curl using your distribution's package manager"
        fi
        exit 1
    fi
    
    print_colored $GREEN "✅ All dependencies found"
}

download_file() {
    local url=$1
    local output=$2
    
    print_colored $YELLOW "🌐 Downloading from: $url"
    
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL -o "$output" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O "$output" "$url"
    else
        print_colored $RED "❌ Neither curl nor wget found"
        return 1
    fi
}

install_laml() {
    print_colored $YELLOW "📦 Installing LAML for Linux..."
    
    # Detect architecture and set appropriate binary URL
    detect_architecture
    print_colored $BLUE "🏗️  Detected architecture: $ARCH_NAME"
    print_colored $BLUE "📥 Using binary: $(basename "$LAML_BINARY_URL")"
    
    # Create directories
    sudo mkdir -p "$INSTALL_DIR"
    mkdir -p "$CONFIG_DIR"
    mkdir -p "$DESKTOP_DIR"
    
    # Download LAML binary
    print_colored $YELLOW "📥 Downloading LAML binary..."
    local temp_file="/tmp/laml_download_$$"
    
    if download_file "$LAML_BINARY_URL" "$temp_file"; then
        # Verify the file was downloaded and has content
        if [ -f "$temp_file" ] && [ -s "$temp_file" ]; then
            local file_size=$(stat -c%s "$temp_file" 2>/dev/null || echo "unknown")
            print_colored $GREEN "✅ LAML binary downloaded successfully ($file_size bytes)"
            
            # Verify it's a valid binary file
            if file "$temp_file" 2>/dev/null | grep -q "ELF.*executable"; then
                print_colored $GREEN "✅ Binary file verification passed"
            else
                print_colored $YELLOW "⚠️  File downloaded but binary verification unclear"
            fi
            
            # Install binary
            if sudo cp "$temp_file" "$INSTALL_DIR/laml" 2>/dev/null; then
                sudo chmod +x "$INSTALL_DIR/laml"
                rm -f "$temp_file"
                print_colored $GREEN "✅ LAML installed to $INSTALL_DIR"
                
                # Test installation
                if "$INSTALL_DIR/laml" version >/dev/null 2>&1; then
                    print_colored $GREEN "✅ LAML is working correctly"
                    
                    # Show version
                    local version_output=$("$INSTALL_DIR/laml" version 2>/dev/null | head -1)
                    print_colored $CYAN "📋 Installed: $version_output"
                else
                    print_colored $YELLOW "⚠️  LAML installed but version check failed"
                    print_colored $YELLOW "💡 Try running: laml version"
                fi
            else
                print_colored $RED "❌ Failed to install binary to $INSTALL_DIR"
                print_colored $YELLOW "💡 Make sure you have sudo privileges"
                rm -f "$temp_file"
                exit 1
            fi
        fi
    else
        print_colored $RED "❌ Failed to download LAML binary"
        print_colored $YELLOW "💡 Please check your internet connection and try again"
        rm -f "$temp_file"
        exit 1
    fi
}

# Create desktop entry
create_desktop_entry() {
    print_colored $YELLOW "🖥️  Creating desktop entry..."
    
    local desktop_file="$DESKTOP_DIR/laml.desktop"
    cat > "$desktop_file" << 'EOF'
[Desktop Entry]
Version=1.0
Type=Application
Name=LAML
Comment=Lightweight, fast compiled programming language
Exec=/usr/local/bin/laml
Icon=text-x-script
Terminal=true
Categories=Development;Programming;
Keywords=programming;language;compiler;
EOF
    
    if [ -f "$desktop_file" ]; then
        chmod +x "$desktop_file"
        print_colored $GREEN "✅ Desktop entry created"
    else
        print_colored $YELLOW "⚠️  Could not create desktop entry"
    fi
}

# Setup shell completion
setup_shell_completion() {
    print_colored $YELLOW "🐚 Setting up shell completion..."
    
    # Check for common shells
    local shell_name=$(basename "$SHELL")
    case "$shell_name" in
        bash)
            local bash_completion="$HOME/.bash_completion"
            echo "complete -C 'laml' laml" >> "$bash_completion"
            print_colored $GREEN "✅ Bash completion added"
            ;;
        zsh)
            local zsh_completion="$HOME/.zshrc"
            echo "autoload -U +X compinit && compinit" >> "$zsh_completion"
            echo "autoload -U +X bashcompinit && bashcompinit" >> "$zsh_completion"
            echo "complete -C 'laml' laml" >> "$zsh_completion"
            print_colored $GREEN "✅ Zsh completion added"
            ;;
        *)
            print_colored $YELLOW "⚠️  Shell completion not set up for $shell_name"
            print_colored $BLUE "💡 You can manually add: complete -C 'laml' laml"
            ;;
    esac
}

# Setup PATH if needed
setup_path() {
    print_colored $YELLOW "🛤️  Setting up PATH..."
    
    # Check if /usr/local/bin is in PATH
    if echo "$PATH" | grep -q "/usr/local/bin"; then
        print_colored $GREEN "✅ /usr/local/bin is already in PATH"
    else
        print_colored $YELLOW "⚠️  /usr/local/bin not found in PATH"
        print_colored $BLUE "💡 You may need to add it to your shell profile"
        
        # Try to add to common profile files
        for profile in "$HOME/.bashrc" "$HOME/.zshrc" "$HOME/.profile"; do
            if [ -f "$profile" ]; then
                echo 'export PATH="/usr/local/bin:$PATH"' >> "$profile"
                print_colored $GREEN "✅ Added to $profile"
                break
            fi
        done
    fi
}

# Main installation function
main() {
    print_header
    
    print_colored $CYAN "🔍 Starting LAML installation for Linux..."
    print_colored $BLUE "📊 System Information:"
    print_colored $BLUE "   OS: $(uname -s)"
    print_colored $BLUE "   Architecture: $(uname -m)"
    print_colored $BLUE "   Kernel: $(uname -r)"
    
    check_linux
    check_dependencies
    
    print_colored $YELLOW "📂 Installation directories:"
    print_colored $BLUE "   Binary: $INSTALL_DIR"
    print_colored $BLUE "   Config: $CONFIG_DIR"
    print_colored $BLUE "   Desktop: $DESKTOP_DIR"
    
    install_laml
    create_desktop_entry
    setup_shell_completion
    setup_path
    
    print_colored $GREEN "🎉 LAML installation completed successfully!"
    print_colored $CYAN "📋 Installation Summary:"
    print_colored $GREEN "   ✅ LAML binary installed to $INSTALL_DIR"
    print_colored $GREEN "   ✅ Desktop entry created"
    print_colored $GREEN "   ✅ Shell completion configured"
    print_colored $GREEN "   ✅ PATH configured"
    
    print_colored $YELLOW "🚀 Getting Started:"
    print_colored $BLUE "   • Run: laml --help"
    print_colored $BLUE "   • Version: laml version"
    print_colored $BLUE "   • Examples: laml run examples/hello.lm"
    
    print_colored $YELLOW "📚 Resources:"
    print_colored $BLUE "   • Documentation: https://github.com/NaveenSingh9999/LAML"
    print_colored $BLUE "   • Examples: $INSTALL_DIR/examples/"
    print_colored $BLUE "   • Config: $CONFIG_DIR"
    
    print_colored $CYAN "💡 Note: You may need to restart your terminal or run 'source ~/.bashrc' to use LAML"
}

# Run the installer
if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    main "$@"
fi
