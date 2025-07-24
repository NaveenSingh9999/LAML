#!/data/data/com.termux/files/usr/bin/bash

# LAML Installer for Termux (Android)
# Specialized installer for Termux environment

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
DESKTOP_DIR="$HOME/.local/share/applications"

print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    print_colored $CYAN "🚀 LAML Installer for Termux"
    print_colored $CYAN "============================"
}

check_termux() {
    if [ ! -d "/data/data/com.termux" ]; then
        print_colored $RED "❌ This installer is designed for Termux only!"
        print_colored $YELLOW "Please use the Linux installer for regular Linux systems."
        exit 1
    fi
    
    print_colored $GREEN "✅ Termux environment detected"
}

detect_android_arch() {
    local arch=$(uname -m)
    case $arch in
        aarch64|arm64)
            echo "arm64"
            ;;
        armv7l|armv8l)
            echo "arm"
            ;;
        x86_64)
            echo "x86_64"
            ;;
        i686)
            echo "x86"
            ;;
        *)
            print_colored $RED "❌ Unsupported architecture: $arch"
            exit 1
            ;;
    esac
}

install_dependencies() {
    print_colored $YELLOW "📦 Installing dependencies..."
    
    # Update package list
    pkg update -y
    
    # Install required packages
    local packages=(
        "golang"          # For building Go-based LAML compiler
        "git"            # For version control
        "curl"           # For downloading
        "unzip"          # For extracting
        "termux-tools"   # Termux utilities
        "file"           # File type detection
    )
    
    for package in "${packages[@]}"; do
        if ! pkg list-installed | grep -q "^$package/"; then
            print_colored $YELLOW "Installing $package..."
            pkg install -y "$package"
        else
            print_colored $GREEN "✅ $package already installed"
        fi
    done
}

install_laml() {
    local arch=$(detect_android_arch)
    print_colored $YELLOW "📦 Installing LAML for Termux ($arch)..."
    
    # Create directories
    mkdir -p "$CONFIG_DIR"
    
    # For demo purposes, create a placeholder binary
    # In production, this would be compiled specifically for Termux
    cat > "$INSTALL_DIR/laml" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo "🚀 LAML Compiler v3.0.0 - Termux"
echo "Architecture: $(uname -m)"
echo "This is a placeholder. In production, this would be the actual LAML binary."
echo ""
echo "Usage: laml [command] [file.lm]"
echo "Commands:"
echo "  run [file]     - Compile and run LAML file"
echo "  compile [file] - Compile LAML file to binary"
echo "  version        - Show version information"
echo "  help           - Show this help message"
echo ""
echo "Termux-specific features:"
echo "  • Android storage access"
echo "  • ARM/ARM64 optimization"
echo "  • Package integration"
EOF
    
    # Make executable
    chmod +x "$INSTALL_DIR/laml"
    
    print_colored $GREEN "✅ LAML binary installed to $INSTALL_DIR"
}

setup_termux_integration() {
    print_colored $YELLOW "🔧 Setting up Termux integration..."
    
    # Create shortcut in Termux home
    cat > "$HOME/laml-dev" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo "🚀 LAML Development Environment"
echo "==============================="
echo "Welcome to LAML on Termux!"
echo ""
echo "Quick commands:"
echo "  laml version      - Check LAML version"
echo "  laml help         - Show help"
echo "  cd ~/storage      - Access Android storage"
echo "  termux-setup-storage - Enable storage access"
echo ""
exec bash
EOF
    
    chmod +x "$HOME/laml-dev"
    
    # Create sample LAML file
    mkdir -p "$HOME/laml-examples"
    cat > "$HOME/laml-examples/hello-termux.lm" << 'EOF'
~ LAML Hello World for Termux
bring xcs.class34;

func main() {
    say "Hello from LAML on Termux!";
    say "Running on Android device";
    
    val device = "Android";
    val environment = "Termux";
    
    say "Device: " + device;
    say "Environment: " + environment;
    
    ~ Loop example
    loop i in 1 to 3 {
        say "Count: " + i;
    }
}
EOF
    
    print_colored $GREEN "✅ Termux integration complete"
    print_colored $BLUE "📁 Sample file created: ~/laml-examples/hello-termux.lm"
}

setup_android_storage() {
    print_colored $YELLOW "📱 Setting up Android storage access..."
    
    if command -v termux-setup-storage >/dev/null 2>&1; then
        echo "This will request storage permission for Termux."
        echo "Please allow storage access when prompted."
        read -p "Continue? (y/N): " -n 1 -r
        echo ""
        
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            termux-setup-storage
            print_colored $GREEN "✅ Storage access configured"
            
            # Create shortcut to storage
            if [ -d "$HOME/storage" ]; then
                ln -sf "$HOME/storage/shared" "$HOME/android-storage" 2>/dev/null || true
                print_colored $GREEN "✅ Android storage accessible at ~/android-storage"
            fi
        else
            print_colored $YELLOW "⚠️  Storage access skipped. You can run 'termux-setup-storage' later."
        fi
    else
        print_colored $YELLOW "⚠️  termux-setup-storage not available"
    fi
}

create_termux_shortcuts() {
    print_colored $YELLOW "⚡ Creating Termux shortcuts..."
    
    # Create shortcut directory
    mkdir -p "$HOME/.shortcuts"
    
    # LAML development shortcut
    cat > "$HOME/.shortcuts/LAML-Dev" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
cd ~/laml-examples
echo "🚀 LAML Development Mode"
echo "========================"
echo "Current directory: $(pwd)"
echo "Available examples:"
ls *.lm 2>/dev/null || echo "No .lm files found"
echo ""
echo "Try: laml run hello-termux.lm"
exec bash
EOF
    
    chmod +x "$HOME/.shortcuts/LAML-Dev"
    
    print_colored $GREEN "✅ Termux shortcuts created"
    print_colored $BLUE "💡 You can now access 'LAML-Dev' from Termux widgets"
}

uninstall_laml() {
    print_colored $YELLOW "🗑️  Uninstalling LAML from Termux..."
    
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
    
    # Remove shortcuts and examples
    if [ -f "$HOME/laml-dev" ]; then
        rm "$HOME/laml-dev"
    fi
    
    if [ -d "$HOME/laml-examples" ]; then
        rm -rf "$HOME/laml-examples"
        print_colored $GREEN "✅ Examples directory removed"
    fi
    
    if [ -f "$HOME/.shortcuts/LAML-Dev" ]; then
        rm "$HOME/.shortcuts/LAML-Dev"
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
    
    print_colored $BLUE "📱 Android architecture: $(detect_android_arch)"
    print_colored $BLUE "📁 Install directory: $INSTALL_DIR"
    print_colored $BLUE "⚙️  Config directory: $CONFIG_DIR"
    
    echo ""
    print_colored $YELLOW "This installer will:"
    print_colored $NC "• Install LAML compiler for Termux"
    print_colored $NC "• Set up Android storage access"
    print_colored $NC "• Create development shortcuts"
    print_colored $NC "• Install example files"
    echo ""
    
    read -p "Continue with installation? (y/N): " -n 1 -r
    echo ""
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_colored $YELLOW "Installation cancelled."
        exit 0
    fi
    
    install_dependencies
    install_laml
    setup_termux_integration
    setup_android_storage
    create_termux_shortcuts
    
    echo ""
    print_colored $GREEN "🎉 LAML installation on Termux completed successfully!"
    echo ""
    print_colored $CYAN "📋 Next steps:"
    print_colored $NC "1. Type 'laml version' to verify installation"
    print_colored $NC "2. Run 'cd ~/laml-examples && laml run hello-termux.lm'"
    print_colored $NC "3. Use 'LAML-Dev' shortcut for quick development"
    print_colored $NC "4. Access Android files via ~/android-storage"
    echo ""
    print_colored $BLUE "📚 Documentation: https://github.com/NaveenSingh9999/LAML"
    print_colored $BLUE "💬 Termux community: https://termux.com"
}

# Run main function
main "$@"
