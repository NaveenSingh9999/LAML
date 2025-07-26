#!/data/data/com.termux/files/usr/bin/bash

# LAML Installer for Termux (Android)
# Downloads and installs LAML from GitHub

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Termux-specific paths
INSTALL_DIR="$PREFIX/bin"
CONFIG_DIR="$HOME/.config/laml"

# GitHub URL for the ARM laml binary for Termux
LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-termux"

print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    print_colored $CYAN "🚀 LAML Installer for Termux"
    print_colored $CYAN "============================"
    print_colored $BLUE "📱 ARM-optimized for Android devices"
    echo
}

check_termux() {
    if [ -z "$PREFIX" ] || [ ! -d "$PREFIX" ]; then
        print_colored $RED "❌ This installer is designed for Termux only!"
        print_colored $YELLOW "Please run this in a Termux terminal on Android."
        exit 1
    fi
    
    print_colored $GREEN "✅ Termux environment detected"
}

install_laml() {
    print_colored $YELLOW "📦 Installing LAML for Termux..."
    
    # Create config directory
    mkdir -p "$CONFIG_DIR"
    
    # Download LAML binary
    print_colored $YELLOW "📥 Downloading ARM-optimized LAML binary..."
    local temp_file="/tmp/laml_download"
    
    if curl -fsSL -o "$temp_file" "$LAML_BINARY_URL"; then
        # Verify the file was downloaded and has content
        if [ -f "$temp_file" ] && [ -s "$temp_file" ]; then
            print_colored $GREEN "✅ LAML binary downloaded successfully"
            
            # Install binary
            cp "$temp_file" "$INSTALL_DIR/laml"
            chmod +x "$INSTALL_DIR/laml"
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
            fi
        else
            print_colored $RED "❌ Downloaded file is empty or corrupted"
            rm -f "$temp_file"
            exit 1
        fi
    else
        print_colored $RED "❌ Failed to download LAML binary"
        print_colored $YELLOW "Please check your internet connection and try again."
        exit 1
    fi
}

create_shortcuts() {
    print_colored $YELLOW "⚡ Creating Termux shortcuts..."
    
    # Create shortcuts directory
    mkdir -p "$HOME/.shortcuts"
    
    # Create laml shortcut
    cat > "$HOME/.shortcuts/laml" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo "🚀 LAML Development Environment"
echo "Type 'laml --help' for usage"
exec bash
EOF
    
    chmod +x "$HOME/.shortcuts/laml"
    print_colored $GREEN "✅ Termux shortcuts created"
    print_colored $BLUE "💡 Use Termux:Widget to add LAML shortcut to home screen"
}

uninstall_laml() {
    print_colored $YELLOW "🗑️  Uninstalling LAML..."
    
    # Remove binary
    if [ -f "$INSTALL_DIR/laml" ]; then
        rm "$INSTALL_DIR/laml"
        print_colored $GREEN "✅ LAML binary removed"
    fi
    
    # Remove config directory
    if [ -d "$CONFIG_DIR" ]; then
        rm -rf "$CONFIG_DIR"
        print_colored $GREEN "✅ Configuration directory removed"
    fi
    
    # Remove shortcuts
    if [ -f "$HOME/.shortcuts/laml" ]; then
        rm "$HOME/.shortcuts/laml"
        print_colored $GREEN "✅ Termux shortcuts removed"
    fi
    
    print_colored $GREEN "🎉 LAML uninstalled successfully!"
}

main() {
    # Check for uninstall flag
    if [ "$1" = "--uninstall" ] || [ "$1" = "-u" ]; then
        uninstall_laml
        exit 0
    fi
    
    print_header
    check_termux
    
    print_colored $BLUE "📁 Install directory: $INSTALL_DIR"
    print_colored $BLUE "⚙️  Config directory: $CONFIG_DIR"
    echo
    
    install_laml
    create_shortcuts
    
    echo
    print_colored $GREEN "🎉 LAML installation completed successfully!"
    echo
    print_colored $CYAN "📋 Quick start:"
    print_colored $NC "• Type 'laml version' to verify installation"
    print_colored $NC "• Type 'laml --help' to see available commands"
    print_colored $NC "• Create a .lm file and run with 'laml run file.lm'"
    echo
    print_colored $BLUE "📚 Documentation: https://github.com/NaveenSingh9999/LAML"
    print_colored $BLUE "🐛 Issues: https://github.com/NaveenSingh9999/LAML/issues"
    echo
    print_colored $CYAN "💡 LAML is now ready to use in Termux!"
}

# Run main function
main "$@"
