#!/bin/bash

# Test script for Linux installer
echo "🧪 Testing Linux Installer Architecture Detection"
echo "================================================"

# Source the installer functions
source installers/linux/install.sh

# Test architecture detection
echo "🔍 Current system:"
echo "   uname -m: $(uname -m)"
echo "   uname -s: $(uname -s)"

# Run detection
detect_architecture

echo ""
echo "🏗️  Detection results:"
echo "   Architecture: $ARCH_NAME"
echo "   Binary URL: $LAML_BINARY_URL"
echo "   Binary name: $(basename "$LAML_BINARY_URL")"

# Verify URL accessibility
echo ""
echo "🌐 Testing URL accessibility:"
if curl -I "$LAML_BINARY_URL" 2>/dev/null | head -1 | grep -q "200"; then
    echo "   ✅ Binary URL is accessible"
else
    echo "   ⚠️  Binary URL check failed (may be expected in test environment)"
fi

echo ""
echo "✅ Linux installer architecture detection test completed"
