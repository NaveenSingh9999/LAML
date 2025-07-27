#!/bin/bash

# LAML v3.3.0 Architecture Support Test
# Tests that all platform-specific binaries work correctly

echo "🧪 Testing LAML v3.3.0 Architecture Support"
echo "============================================="
echo

# Test current system binary
echo "1. Testing current system binary:"
if command -v ./laml &> /dev/null; then
    echo "   ✅ LAML binary found"
    echo "   📍 Version: $(./laml version 2>/dev/null || echo 'Failed to get version')"
    
    # Test with simple program
    echo 'bring xcs.class34; func main() { say "Architecture test passed!"; }' > test_arch.lm
    if ./laml run test_arch.lm 2>/dev/null; then
        echo "   ✅ LAML execution test passed"
    else
        echo "   ❌ LAML execution test failed"
    fi
    rm -f test_arch.lm
else
    echo "   ❌ LAML binary not found in current directory"
fi
echo

# Test architecture-specific binaries
echo "2. Testing architecture-specific binaries:"

# Test x86_64 binary (if available)
if [ -f "./laml-linux-x86_64" ]; then
    echo "   🔍 Testing x86_64 binary:"
    file_info=$(file ./laml-linux-x86_64)
    echo "   📄 File info: $file_info"
    if echo "$file_info" | grep -q "x86-64"; then
        echo "   ✅ x86_64 binary architecture correct"
    else
        echo "   ❌ x86_64 binary architecture incorrect"
    fi
fi

# Test ARM64 binary (if available)
if [ -f "./laml-linux-arm64" ]; then
    echo "   🔍 Testing ARM64 binary:"
    file_info=$(file ./laml-linux-arm64)
    echo "   📄 File info: $file_info"
    if echo "$file_info" | grep -q "aarch64"; then
        echo "   ✅ ARM64 binary architecture correct"
    else
        echo "   ❌ ARM64 binary architecture incorrect"
    fi
fi

# Test ARMv7 binary (if available)
if [ -f "./laml-linux-armv7" ]; then
    echo "   🔍 Testing ARMv7 binary:"
    file_info=$(file ./laml-linux-armv7)
    echo "   📄 File info: $file_info"
    if echo "$file_info" | grep -q "ARM"; then
        echo "   ✅ ARMv7 binary architecture correct"
    else
        echo "   ❌ ARMv7 binary architecture incorrect"
    fi
fi

# Test Termux binary (if available)
if [ -f "./laml-termux" ]; then
    echo "   🔍 Testing Termux binary:"
    file_info=$(file ./laml-termux)
    echo "   📄 File info: $file_info"
    if echo "$file_info" | grep -q "ARM"; then
        echo "   ✅ Termux binary architecture correct (ARM)"
    else
        echo "   ❌ Termux binary architecture incorrect"
    fi
fi
echo

# Test current system architecture detection
echo "3. System architecture detection:"
current_arch=$(uname -m)
echo "   🖥️  Current architecture: $current_arch"

case "$current_arch" in
    x86_64|amd64)
        expected_binary="laml-linux-x86_64"
        ;;
    aarch64|arm64)
        expected_binary="laml-linux-arm64"
        ;;
    armv7l|armv6l)
        expected_binary="laml-linux-armv7"
        ;;
    *)
        expected_binary="unknown"
        ;;
esac

if [ "$expected_binary" != "unknown" ]; then
    echo "   🎯 Expected binary: $expected_binary"
    if [ -f "./$expected_binary" ]; then
        echo "   ✅ Correct binary available for this architecture"
    else
        echo "   ❌ Binary for this architecture not found"
    fi
else
    echo "   ⚠️  Unknown architecture - may need special handling"
fi
echo

# Test installer URLs
echo "4. Testing installer accessibility:"
installers=(
    "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh"
    "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/macos/install.sh"
    "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/termux/install.sh"
    "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1"
)

for url in "${installers[@]}"; do
    platform=$(echo "$url" | sed 's/.*\/\([^/]*\)\/install\..*/\1/')
    echo "   🌐 Testing $platform installer..."
    if curl -s -I "$url" | grep -q "200 OK"; then
        echo "   ✅ $platform installer accessible"
    else
        echo "   ❌ $platform installer not accessible"
    fi
done
echo

echo "🏁 Architecture Support Test Complete"
echo "====================================="
