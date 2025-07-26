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

# GitHub URL for the universal laml binary
# GitHub URLs  
LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml"

print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    print_colored $CYAN "🚀 LAML Installer for Termux"
    print_colored $CYAN "============================"
    print_colored $BLUE "📥 Downloads latest LAML from GitHub"
    echo
}

check_termux() {
    if [ ! -d "/data/data/com.termux" ]; then
        print_colored $RED "❌ This installer is designed for Termux only!"
        print_colored $YELLOW "Please use the Linux installer for regular Linux systems."
        exit 1
    fi
    
    print_colored $GREEN "✅ Termux environment detected"
}

check_dependencies() {
    print_colored $YELLOW "🔍 Checking dependencies..."
    
    local missing_deps=()
    
    if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
        missing_deps+=("curl")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_colored $YELLOW "📦 Installing missing dependencies..."
        
        for dep in "${missing_deps[@]}"; do
            print_colored $YELLOW "Installing $dep..."
            pkg install -y "$dep"
        done
    fi
    
    print_colored $GREEN "✅ All dependencies ready"
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
    print_colored $YELLOW "📦 Downloading and installing LAML from GitHub..."
    
    # Create directories
    mkdir -p "$INSTALL_DIR"
    mkdir -p "$CONFIG_DIR"
    
    # Download LAML binary
    print_colored $YELLOW "📥 Downloading LAML binary..."
    local temp_file="$HOME/laml_download"
    
    if download_file "$LAML_BINARY_URL" "$temp_file"; then
        # Verify the file was downloaded and has content
        if [ -f "$temp_file" ] && [ -s "$temp_file" ]; then
            print_colored $GREEN "✅ LAML binary downloaded successfully"
            
            # Install binary
            cp "$temp_file" "$INSTALL_DIR/laml"
            chmod +x "$INSTALL_DIR/laml"
            rm -f "$temp_file"
            
            print_colored $GREEN "✅ LAML binary installed as 'laml' to $INSTALL_DIR"
            
            # Test installation
            if "$INSTALL_DIR/laml" version >/dev/null 2>&1; then
                print_colored $GREEN "✅ LAML is working correctly"
            else
                print_colored $YELLOW "⚠️  LAML installed but may need configuration"
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

setup_termux_shortcuts() {
    print_colored $YELLOW "📱 Setting up Termux shortcuts..."
    
    local shortcuts_dir="$HOME/.shortcuts"
    mkdir -p "$shortcuts_dir"
    
    # Create LAML shortcut
    cat > "$shortcuts_dir/laml" << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo "LAML Development Environment"
echo "Type 'laml --help' for usage"
echo ""
cd "$HOME"
exec bash
EOF
    
    chmod +x "$shortcuts_dir/laml"
    print_colored $GREEN "✅ Termux shortcuts created"
}

setup_shell_completion() {
    print_colored $YELLOW "🔧 Setting up shell completion..."
    
    # Bash completion
    local bash_completion_dir="$HOME/.local/share/bash-completion/completions"
    mkdir -p "$bash_completion_dir"
    
    cat > "$bash_completion_dir/laml" << 'EOF'
_laml_completion() {
    local cur prev commands
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    commands="run compile version help"
    
    if [[ ${cur} == -* ]]; then
        COMPREPLY=($(compgen -W "--help --version" -- ${cur}))
    elif [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=($(compgen -W "${commands}" -- ${cur}))
    elif [[ ${prev} == "run" || ${prev} == "compile" ]]; then
        COMPREPLY=($(compgen -f -X "!*.lm" -- ${cur}))
    fi
}
complete -F _laml_completion laml
EOF
    
    # Add to bashrc
    if ! grep -q "laml completion" "$HOME/.bashrc" 2>/dev/null; then
        echo "" >> "$HOME/.bashrc"
        echo "# LAML completion" >> "$HOME/.bashrc"
        echo "[ -f $bash_completion_dir/laml ] && source $bash_completion_dir/laml" >> "$HOME/.bashrc"
    fi
    
    print_colored $GREEN "✅ Shell completion installed"
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
    
    # Remove bash completion
    if [ -f "$HOME/.local/share/bash-completion/completions/laml" ]; then
        rm "$HOME/.local/share/bash-completion/completions/laml"
        print_colored $GREEN "✅ Shell completion removed"
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
    
    echo ""
    read -p "Continue with installation? (y/N): " -n 1 -r
    echo ""
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_colored $YELLOW "Installation cancelled."
        exit 0
    fi
    
    check_dependencies
    install_laml
    setup_termux_shortcuts
    setup_shell_completion
    
    echo ""
    print_colored $GREEN "🎉 LAML installation completed successfully!"
    echo ""
    print_colored $CYAN "📋 Next steps:"
    print_colored $NC "1. Restart Termux or run: source ~/.bashrc"
    print_colored $NC "2. Type 'laml version' to verify installation"
    print_colored $NC "3. Type 'laml --help' to see available commands"
    print_colored $NC "4. Create your first .lm file and run with 'laml run file.lm'"
    echo ""
    print_colored $BLUE "📚 Documentation: https://github.com/NaveenSingh9999/LAML"
    print_colored $BLUE "🐛 Issues: https://github.com/NaveenSingh9999/LAML/issues"
    echo ""
    print_colored $CYAN "💡 You can now use 'laml' from anywhere in Termux!"
    print_colored $CYAN "💡 Check the Termux shortcuts widget for quick access!"
}

# Run main function
main "$@"
