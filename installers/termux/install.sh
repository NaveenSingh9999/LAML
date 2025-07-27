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
LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-linux-arm64"

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
    
    # Check for curl
    if ! command -v curl >/dev/null 2>&1; then
        print_colored $RED "❌ curl is required but not installed"
        print_colored $YELLOW "💡 Install it with: pkg install curl"
        exit 1
    fi
    
    print_colored $GREEN "✅ curl is available"
    
    # Check write permissions
    if [ ! -w "$PREFIX/bin" ]; then
        print_colored $RED "❌ No write permission to $PREFIX/bin"
        print_colored $YELLOW "💡 This is unusual for Termux. Try restarting Termux."
        exit 1
    fi
    
    print_colored $GREEN "✅ Write permissions verified"
}

install_laml() {
    print_colored $YELLOW "📦 Installing LAML for Termux..."
    
    # Create config directory
    mkdir -p "$CONFIG_DIR"
    
    # Use Termux-appropriate temp directory
    local temp_dir="$PREFIX/tmp"
    local temp_file="$temp_dir/laml_download_$$"
    
    # Ensure temp directory exists
    if ! mkdir -p "$temp_dir" 2>/dev/null; then
        # Fallback to home directory if PREFIX/tmp fails
        temp_dir="$HOME"
        temp_file="$temp_dir/.laml_download_$$"
        print_colored $YELLOW "⚠️  Using home directory for temporary file"
    fi
    
    print_colored $YELLOW "📥 Downloading ARM-optimized LAML binary..."
    print_colored $BLUE "🌐 From: $LAML_BINARY_URL"
    print_colored $BLUE "📁 Temp: $temp_file"
    
    # Download with better error handling
    if curl -fsSL -o "$temp_file" "$LAML_BINARY_URL" 2>/dev/null; then
        # Verify the file was downloaded and has content
        if [ -f "$temp_file" ] && [ -s "$temp_file" ]; then
            local file_size=$(stat -c%s "$temp_file" 2>/dev/null || wc -c < "$temp_file" 2>/dev/null || echo "unknown")
            print_colored $GREEN "✅ LAML binary downloaded successfully ($file_size bytes)"
            
            # Verify it's a valid binary file
            if file "$temp_file" 2>/dev/null | grep -q "ELF.*executable"; then
                print_colored $GREEN "✅ Binary file verification passed"
            else
                print_colored $YELLOW "⚠️  File downloaded but binary verification unclear"
            fi
            
            # Install binary with error checking
            if cp "$temp_file" "$INSTALL_DIR/laml" 2>/dev/null; then
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
                    print_colored $YELLOW "💡 Try running: laml version"
                fi
            else
                print_colored $RED "❌ Failed to copy binary to $INSTALL_DIR"
                print_colored $YELLOW "💡 Check if you have write permissions to $INSTALL_DIR"
                rm -f "$temp_file"
                exit 1
            fi
        else
            print_colored $RED "❌ Downloaded file is empty or corrupted"
            print_colored $YELLOW "💡 File: $temp_file"
            if [ -f "$temp_file" ]; then
                local file_size=$(stat -c%s "$temp_file" 2>/dev/null || echo "0")
                print_colored $YELLOW "💡 File size: $file_size bytes"
            fi
            rm -f "$temp_file"
            exit 1
        fi
    else
        print_colored $RED "❌ Failed to download LAML binary"
        print_colored $YELLOW "💡 Check your internet connection and try again"
        print_colored $YELLOW "💡 Temp directory: $temp_dir"
        print_colored $YELLOW "💡 URL: $LAML_BINARY_URL"
        exit 1
    fi
}

create_shortcuts() {
    print_colored $YELLOW "⚡ Creating Termux shortcuts..."
    
    # Create shortcuts directory
    if mkdir -p "$HOME/.shortcuts" 2>/dev/null; then
        # Create laml shortcut
        cat > "$HOME/.shortcuts/laml" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo "🚀 LAML Development Environment"
echo "Type 'laml --help' for usage"
exec bash
EOF
        
        if chmod +x "$HOME/.shortcuts/laml" 2>/dev/null; then
            print_colored $GREEN "✅ Termux shortcuts created"
            print_colored $BLUE "💡 Use Termux:Widget to add LAML shortcut to home screen"
        else
            print_colored $YELLOW "⚠️  Shortcut created but chmod failed"
        fi
    else
        print_colored $YELLOW "⚠️  Could not create shortcuts directory"
    fi
}

test_installation() {
    print_colored $YELLOW "🧪 Testing LAML installation..."
    
    # Test basic command
    if command -v laml >/dev/null 2>&1; then
        print_colored $GREEN "✅ laml command is available in PATH"
    else
        print_colored $YELLOW "⚠️  laml not in PATH, but binary exists at $INSTALL_DIR/laml"
    fi
    
    # Test version command
    if "$INSTALL_DIR/laml" version >/dev/null 2>&1; then
        local version_output=$("$INSTALL_DIR/laml" version 2>/dev/null | head -1)
        print_colored $GREEN "✅ Version check passed: $version_output"
    else
        print_colored $RED "❌ Version check failed"
        return 1
    fi
    
    # Test simple program compilation (optional)
    local test_file="$HOME/.laml_test_$$"
    echo 'bring xcs.class34; func main() { say "Test successful!"; }' > "$test_file.lm"
    
    if "$INSTALL_DIR/laml" run "$test_file.lm" >/dev/null 2>&1; then
        print_colored $GREEN "✅ Program execution test passed"
    else
        print_colored $YELLOW "⚠️  Program execution test failed (this may be normal)"
    fi
    
    rm -f "$test_file.lm" 2>/dev/null
    return 0
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
    # Check for debug flag
    if [ "$1" = "--debug" ] || [ "$1" = "-d" ]; then
        set -x
        shift
    fi
    
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
    test_installation
    
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
    
    # Show PATH info
    print_colored $BLUE "💡 Add $INSTALL_DIR to your PATH if needed:"
    print_colored $NC "  echo 'export PATH=\$PATH:$INSTALL_DIR' >> ~/.bashrc"
}

# Run main function
main "$@"
