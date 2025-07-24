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
