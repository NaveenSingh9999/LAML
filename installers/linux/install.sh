#!/bin/bash

# LAML Installer for Linux (x86_64 and ARM64)
# One-click installer script

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

print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    print_colored $CYAN "🚀 LAML Installer for Linux"
    print_colored $CYAN "==========================="
}

detect_architecture() {
    local arch=$(uname -m)
    case $arch in
        x86_64)
            echo "x86_64"
            ;;
        aarch64|arm64)
            echo "arm64"
            ;;
        armv7l)
            echo "armv7"
            ;;
        *)
            print_colored $RED "❌ Unsupported architecture: $arch"
            exit 1
            ;;
    esac
}

check_dependencies() {
    print_colored $YELLOW "🔍 Checking dependencies..."
    
    local missing_deps=()
    
    # Check for required tools
    command -v curl >/dev/null 2>&1 || missing_deps+=("curl")
    command -v unzip >/dev/null 2>&1 || missing_deps+=("unzip")
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_colored $YELLOW "📦 Installing missing dependencies: ${missing_deps[*]}"
        
        if command -v apt-get >/dev/null 2>&1; then
            sudo apt-get update && sudo apt-get install -y "${missing_deps[@]}"
        elif command -v yum >/dev/null 2>&1; then
            sudo yum install -y "${missing_deps[@]}"
        elif command -v pacman >/dev/null 2>&1; then
            sudo pacman -S --noconfirm "${missing_deps[@]}"
        else
            print_colored $RED "❌ Unable to install dependencies automatically."
            print_colored $YELLOW "Please install manually: ${missing_deps[*]}"
            exit 1
        fi
    fi
}

install_laml() {
    local arch=$(detect_architecture)
    print_colored $YELLOW "📦 Installing LAML for $arch..."
    
    # Create directories
    sudo mkdir -p "$INSTALL_DIR"
    mkdir -p "$CONFIG_DIR"
    mkdir -p "$DESKTOP_DIR"
    
# Install LAML binary
echo -e "${YELLOW}📦 Installing LAML compiler...${NC}"

# Check if binary exists in package
if [ -f "${script_dir}/laml" ]; then
    cp "${script_dir}/laml" "${install_dir}/laml"
    chmod +x "${install_dir}/laml"
    echo -e "${GREEN}✅ LAML binary installed${NC}"
else
    echo -e "${RED}❌ LAML binary not found in package!${NC}"
    exit 1
fi    # Install binary
    sudo cp /tmp/laml "$INSTALL_DIR/laml"
    sudo chmod +x "$INSTALL_DIR/laml"
    
    print_colored $GREEN "✅ LAML binary installed to $INSTALL_DIR"
}

create_desktop_entry() {
    print_colored $YELLOW "🖥️  Creating desktop entry..."
    
    cat > "$DESKTOP_DIR/laml-terminal.desktop" << EOF
[Desktop Entry]
Name=LAML Terminal
Comment=LAML Development Environment
Exec=gnome-terminal --title="LAML Terminal" -- bash -c "echo 'LAML Development Environment'; echo 'Type laml --help for usage'; bash"
Icon=terminal
Type=Application
Categories=Development;Programming;
Keywords=LAML;Programming;Compiler;
StartupNotify=true
EOF
    
    chmod +x "$DESKTOP_DIR/laml-terminal.desktop"
    print_colored $GREEN "✅ Desktop entry created"
}

install_vscode_extension() {
    if command -v code >/dev/null 2>&1; then
        print_colored $YELLOW "📝 Installing VS Code extension..."
        
        # Look for VSIX file
        local vsix_file=$(find "$(dirname "$0")/.." -name "laml-*.vsix" | head -1)
        
        if [ -n "$vsix_file" ] && [ -f "$vsix_file" ]; then
            code --install-extension "$vsix_file"
            print_colored $GREEN "✅ VS Code extension installed"
        else
            print_colored $YELLOW "⚠️  VS Code extension not found. You can install it manually later."
        fi
    else
        print_colored $YELLOW "ℹ️  VS Code not found. Extension installation skipped."
    fi
}

setup_shell_completion() {
    print_colored $YELLOW "🔧 Setting up shell completion..."
    
    # Bash completion
    if [ -n "$BASH_VERSION" ]; then
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
        print_colored $GREEN "✅ Bash completion installed"
    fi
}

uninstall_laml() {
    print_colored $YELLOW "🗑️  Uninstalling LAML..."
    
    # Remove binary
    if [ -f "$INSTALL_DIR/laml" ]; then
        sudo rm "$INSTALL_DIR/laml"
        print_colored $GREEN "✅ LAML binary removed"
    fi
    
    # Remove config directory
    if [ -d "$CONFIG_DIR" ]; then
        rm -rf "$CONFIG_DIR"
        print_colored $GREEN "✅ Configuration directory removed"
    fi
    
    # Remove desktop entry
    if [ -f "$DESKTOP_DIR/laml-terminal.desktop" ]; then
        rm "$DESKTOP_DIR/laml-terminal.desktop"
        print_colored $GREEN "✅ Desktop entry removed"
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
    
    # Check for root privileges for installation
    if [ "$EUID" -eq 0 ]; then
        print_colored $RED "❌ Don't run this installer as root!"
        print_colored $YELLOW "The installer will ask for sudo when needed."
        exit 1
    fi
    
    print_colored $BLUE "📋 Detected architecture: $(detect_architecture)"
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
    create_desktop_entry
    install_vscode_extension
    setup_shell_completion
    
    echo ""
    print_colored $GREEN "🎉 LAML installation completed successfully!"
    echo ""
    print_colored $CYAN "📋 Next steps:"
    print_colored $NC "1. Restart your terminal or run: source ~/.bashrc"
    print_colored $NC "2. Type 'laml version' to verify installation"
    print_colored $NC "3. Create your first .lm file and run with 'laml run file.lm'"
    echo ""
    print_colored $BLUE "📚 Documentation: https://github.com/NaveenSingh9999/LAML"
    print_colored $BLUE "🐛 Issues: https://github.com/NaveenSingh9999/LAML/issues"
}

# Run main function
main "$@"
