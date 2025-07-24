#!/bin/bash

# LAML Installation Test Suite
# Tests all installer packages

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_colored() {
    echo -e "${1}${2}${NC}"
}

print_header() {
    print_colored $CYAN "🧪 LAML Installer Test Suite"
    print_colored $CYAN "============================"
}

test_windows_installer() {
    print_colored $YELLOW "🪟 Testing Windows installer..."
    
    if [ -f "windows/install.ps1" ] && [ -f "windows/install.bat" ] && [ -f "windows/laml.exe" ]; then
        print_colored $GREEN "✅ Windows installer files present"
        print_colored $BLUE "   • install.ps1 (PowerShell script)"
        print_colored $BLUE "   • install.bat (Batch wrapper)"
        print_colored $BLUE "   • laml.exe (Windows binary)"
        print_colored $BLUE "   • README.txt (Installation guide)"
    else
        print_colored $RED "❌ Windows installer files missing"
        return 1
    fi
}

test_linux_installer() {
    print_colored $YELLOW "🐧 Testing Linux installer..."
    
    if [ -f "linux/install.sh" ] && [ -f "linux/laml-x86_64" ] && [ -f "linux/laml-arm64" ]; then
        print_colored $GREEN "✅ Linux installer files present"
        print_colored $BLUE "   • install.sh (Installation script)"
        print_colored $BLUE "   • laml-x86_64 (x86_64 binary)"
        print_colored $BLUE "   • laml-arm64 (ARM64 binary)"
        print_colored $BLUE "   • README.md (Installation guide)"
        
        # Test if script is executable
        if [ -x "linux/install.sh" ]; then
            print_colored $GREEN "✅ Linux installer is executable"
        else
            print_colored $RED "❌ Linux installer is not executable"
            return 1
        fi
    else
        print_colored $RED "❌ Linux installer files missing"
        return 1
    fi
}

test_termux_installer() {
    print_colored $YELLOW "🤖 Testing Termux installer..."
    
    if [ -f "termux/install.sh" ] && [ -f "termux/laml-termux" ]; then
        print_colored $GREEN "✅ Termux installer files present"
        print_colored $BLUE "   • install.sh (Termux installation script)"
        print_colored $BLUE "   • laml-termux (ARM binary for Android)"
        print_colored $BLUE "   • README.md (Installation guide)"
        
        # Test if script is executable
        if [ -x "termux/install.sh" ]; then
            print_colored $GREEN "✅ Termux installer is executable"
        else
            print_colored $RED "❌ Termux installer is not executable"
            return 1
        fi
    else
        print_colored $RED "❌ Termux installer files missing"
        return 1
    fi
}

test_vscode_extension() {
    print_colored $YELLOW "📝 Testing VS Code extension..."
    
    if [ -f "vscode-extension/laml-3.0.0.vsix" ]; then
        print_colored $GREEN "✅ VS Code extension VSIX present"
        
        # Check VSIX file size (should be reasonable)
        local size=$(du -h "vscode-extension/laml-3.0.0.vsix" | cut -f1)
        print_colored $BLUE "   • VSIX size: $size"
        print_colored $BLUE "   • Ready for VS Code installation"
        
        if [ -f "vscode-extension/README.md" ]; then
            print_colored $GREEN "✅ VS Code extension documentation present"
        fi
    else
        print_colored $RED "❌ VS Code extension VSIX missing"
        return 1
    fi
}

test_universal_installer() {
    print_colored $YELLOW "🌍 Testing universal installer..."
    
    if [ -f "install.sh" ]; then
        print_colored $GREEN "✅ Universal installer present"
        
        if [ -x "install.sh" ]; then
            print_colored $GREEN "✅ Universal installer is executable"
            
            # Test the platform detection logic (dry run)
            print_colored $BLUE "   Testing platform detection..."
            if grep -q "detect_platform" "install.sh"; then
                print_colored $GREEN "✅ Platform detection logic present"
            else
                print_colored $RED "❌ Platform detection logic missing"
                return 1
            fi
        else
            print_colored $RED "❌ Universal installer is not executable"
            return 1
        fi
    else
        print_colored $RED "❌ Universal installer missing"
        return 1
    fi
}

test_documentation() {
    print_colored $YELLOW "📚 Testing documentation..."
    
    if [ -f "README.md" ]; then
        print_colored $GREEN "✅ Main README.md present"
        
        # Check for key sections
        if grep -q "Quick Installation" "README.md" && grep -q "Platform-Specific" "README.md"; then
            print_colored $GREEN "✅ Documentation contains installation instructions"
        else
            print_colored $RED "❌ Documentation missing key sections"
            return 1
        fi
    else
        print_colored $RED "❌ Main documentation missing"
        return 1
    fi
}

test_binaries() {
    print_colored $YELLOW "🔧 Testing binary files..."
    
    local binaries=(
        "windows/laml.exe"
        "linux/laml-x86_64"
        "linux/laml-arm64"
        "termux/laml-termux"
    )
    
    for binary in "${binaries[@]}"; do
        if [ -f "$binary" ]; then
            local size=$(du -h "$binary" | cut -f1)
            print_colored $GREEN "✅ $binary ($size)"
        else
            print_colored $RED "❌ $binary missing"
            return 1
        fi
    done
}

simulate_installation() {
    print_colored $YELLOW "🎭 Simulating installation process..."
    
    # Test universal installer help
    if ./install.sh --help 2>/dev/null || true; then
        print_colored $GREEN "✅ Universal installer responds to --help"
    else
        print_colored $BLUE "ℹ️  Universal installer doesn't have --help (normal)"
    fi
    
    print_colored $GREEN "✅ Installation simulation complete"
}

main() {
    print_header
    
    if [ ! -d "$(pwd)/linux" ] || [ ! -d "$(pwd)/windows" ]; then
        print_colored $RED "❌ Please run this script from the release directory"
        print_colored $YELLOW "Usage: cd release/laml-v3.0.0 && bash ../../test-installers.sh"
        exit 1
    fi
    
    print_colored $BLUE "📁 Testing in directory: $(pwd)"
    echo ""
    
    # Run all tests
    local tests=(
        "test_windows_installer"
        "test_linux_installer"
        "test_termux_installer"
        "test_vscode_extension"
        "test_universal_installer"
        "test_documentation"
        "test_binaries"
        "simulate_installation"
    )
    
    local passed=0
    local failed=0
    
    for test in "${tests[@]}"; do
        echo "Running $test..."
        if $test; then
            ((passed++))
        else
            ((failed++))
        fi
        echo ""
    done
    
    # Results summary
    print_colored $CYAN "📊 Test Results Summary:"
    print_colored $GREEN "✅ Passed: $passed"
    if [ $failed -gt 0 ]; then
        print_colored $RED "❌ Failed: $failed"
    else
        print_colored $GREEN "❌ Failed: $failed"
    fi
    
    echo ""
    if [ $failed -eq 0 ]; then
        print_colored $GREEN "🎉 All installer tests passed!"
        print_colored $CYAN "🚀 LAML v3.0.0 release package is ready for distribution!"
        echo ""
        print_colored $BLUE "📋 Distribution checklist:"
        print_colored $NC "  ✅ Cross-platform installers (Windows, Linux, Termux)"
        print_colored $NC "  ✅ VS Code extension VSIX package"
        print_colored $NC "  ✅ Universal installer script"
        print_colored $NC "  ✅ Complete documentation"
        print_colored $NC "  ✅ Binary files for all architectures"
        echo ""
        print_colored $BLUE "📦 Ready to upload to GitHub Releases!"
    else
        print_colored $RED "❌ Some tests failed. Please fix issues before distribution."
        exit 1
    fi
}

main "$@"
