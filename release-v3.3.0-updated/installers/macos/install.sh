#!/bin/bash

# LAML Installer for macOS
# Downloads and installs LAML from GitHub (universal binary)

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

# GitHub URL for the universal laml binary
# GitHub URLs
detect_architecture() {
    local arch=$(uname -m)
    case $arch in
        x86_64)
            LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-linux-x86_64"
            ;;
        arm64)
            LAML_BINARY_URL="https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-linux-arm64"
            ;;
        *)
            print_colored $RED "❌ Unsupported macOS architecture: $arch"
            print_colored $YELLOW "Supported architectures: x86_64 (Intel), arm64 (Apple Silicon)"
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
    print_colored $CYAN "🚀 LAML Installer for macOS"
    print_colored $CYAN "=========================="
    print_colored $BLUE "📥 Downloads latest LAML from GitHub"
    echo
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
        print_colored $YELLOW "You can install curl with Homebrew: brew install curl"
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
    print_colored $YELLOW "📦 Downloading and installing LAML from GitHub..."
    
    # Detect architecture and set appropriate binary URL
    detect_architecture
    print_colored $BLUE "🏗️  Detected architecture: $(uname -m)"
    print_colored $BLUE "📥 Using binary: $(basename "$LAML_BINARY_URL")"
    
    # Create directories
    sudo mkdir -p "$INSTALL_DIR"
    mkdir -p "$CONFIG_DIR"
    
    # Download LAML binary
    print_colored $YELLOW "📥 Downloading LAML binary..."
    local temp_file="$HOME/laml_download"
    
    if download_file "$LAML_BINARY_URL" "$temp_file"; then
        # Verify the file was downloaded and has content
        if [ -f "$temp_file" ] && [ -s "$temp_file" ]; then
            print_colored $GREEN "✅ LAML binary downloaded successfully"
            
            # Install binary
            sudo cp "$temp_file" "$INSTALL_DIR/laml"
            sudo chmod +x "$INSTALL_DIR/laml"
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

install_vscode_extension() {
    if command -v code >/dev/null 2>&1; then
        print_colored $YELLOW "📝 Checking for VS Code extension..."
        
        # Try to find the extension in common locations
        local vsix_locations=(
            "$(dirname "$0")/../laml-*.vsix"
            "$(dirname "$0")/../../vscode-extension/laml-*.vsix"
            "$(dirname "$0")/../../installers/laml-*.vsix"
        )
        
        local vsix_file=""
        for location in "${vsix_locations[@]}"; do
            if ls $location 1> /dev/null 2>&1; then
                vsix_file=$(ls $location | head -1)
                break
            fi
        done
        
        if [ -n "$vsix_file" ] && [ -f "$vsix_file" ]; then
            code --install-extension "$vsix_file"
            print_colored $GREEN "✅ VS Code extension installed"
        else
            print_colored $BLUE "ℹ️  VS Code extension not found in installer package"
        fi
    else
        print_colored $BLUE "ℹ️  VS Code not found - extension installation skipped"
    fi
}

setup_shell_completion() {
    print_colored $YELLOW "🔧 Setting up shell completion..."
    
    # Bash completion (if bash is available)
    if command -v bash >/dev/null 2>&1; then
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
    
    # Zsh completion (if zsh is available and oh-my-zsh is installed)
    if command -v zsh >/dev/null 2>&1 && [ -d "$HOME/.oh-my-zsh" ]; then
        local zsh_completion_dir="$HOME/.oh-my-zsh/custom"
        mkdir -p "$zsh_completion_dir"
        
        cat > "$zsh_completion_dir/_laml" << 'EOF'
#compdef laml

_laml() {
    local context state line
    _arguments \
        '1:command:(run compile version help)' \
        '*::arg:->args'
    
    case $state in
        args)
            case $words[1] in
                run|compile)
                    _files -g "*.lm"
                    ;;
            esac
            ;;
    esac
}

_laml "$@"
EOF
        print_colored $GREEN "✅ Zsh completion installed"
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
    
    # Remove bash completion
    if [ -f "$HOME/.local/share/bash-completion/completions/laml" ]; then
        rm "$HOME/.local/share/bash-completion/completions/laml"
        print_colored $GREEN "✅ Bash completion removed"
    fi
    
    # Remove zsh completion
    if [ -f "$HOME/.oh-my-zsh/custom/_laml" ]; then
        rm "$HOME/.oh-my-zsh/custom/_laml"
        print_colored $GREEN "✅ Zsh completion removed"
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
    install_vscode_extension
    setup_shell_completion
    
    echo ""
    print_colored $GREEN "🎉 LAML installation completed successfully!"
    echo ""
    print_colored $CYAN "📋 Next steps:"
    print_colored $NC "1. Restart your terminal or run: source ~/.bashrc (or ~/.zshrc)"
    print_colored $NC "2. Type 'laml version' to verify installation"
    print_colored $NC "3. Type 'laml --help' to see available commands"
    print_colored $NC "4. Create your first .lm file and run with 'laml run file.lm'"
    echo ""
    print_colored $BLUE "📚 Documentation: https://github.com/NaveenSingh9999/LAML"
    print_colored $BLUE "🐛 Issues: https://github.com/NaveenSingh9999/LAML/issues"
    echo ""
    print_colored $CYAN "💡 You can now use 'laml' from anywhere in your terminal!"
}

# Run main function
main "$@"
