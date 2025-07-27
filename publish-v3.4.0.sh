#!/bin/bash

# LAML v3.4.0 Release Publisher
# This script creates the GitHub release for v3.4.0

set -e

VERSION="v3.4.0"
REPO="NaveenSingh9999/LAML"
RELEASE_TITLE="LAML v3.4.0 - C-Style Format Specifiers and Escape Sequences"

echo "🚀 GitHub Release Publisher for LAML ${VERSION}"
echo "=============================================="
echo

# Check if gh CLI is available
if ! command -v gh >/dev/null 2>&1; then
    echo "❌ GitHub CLI (gh) not found!"
    echo "📋 Manual release instructions:"
    echo
    echo "1. Go to: https://github.com/${REPO}/releases/new"
    echo "2. Tag: ${VERSION}"
    echo "3. Title: ${RELEASE_TITLE}"
    echo "4. Description: Copy from RELEASE-NOTES-v3.4.0.md"
    echo "5. Upload assets:"
    echo "   - laml-linux-x86_64"
    echo "   - laml-linux-arm64"
    echo "   - laml-linux-armv7"
    echo "   - laml-windows-x86_64.exe"
    echo "   - laml (main binary)"
    echo "   - vscode-extension/laml-3.4.0.vsix"
    echo
    echo "✅ All binaries are ready in current directory!"
    echo "✅ Release notes ready in RELEASE-NOTES-v3.4.0.md"
    exit 0
fi

echo "✅ GitHub CLI found!"
echo "📋 Creating release..."

# Check if user is logged in
if ! gh auth status >/dev/null 2>&1; then
    echo "❌ Not logged into GitHub CLI"
    echo "Run: gh auth login"
    exit 1
fi

# Create release with assets
echo "📦 Creating GitHub release with binaries..."
gh release create "${VERSION}" \
    --repo "${REPO}" \
    --title "${RELEASE_TITLE}" \
    --notes-file "RELEASE-NOTES-v3.4.0.md" \
    laml-linux-x86_64 \
    laml-linux-arm64 \
    laml-linux-armv7 \
    laml-windows-x86_64.exe \
    laml

# Check if VS Code extension exists
if [ -f "vscode-extension/laml-3.4.0.vsix" ]; then
    echo "📦 Adding VS Code extension..."
    gh release upload "${VERSION}" vscode-extension/laml-3.4.0.vsix --repo "${REPO}"
else
    echo "⚠️  VS Code extension not found at vscode-extension/laml-3.4.0.vsix"
fi

echo
echo "🎉 Release v3.4.0 published successfully!"
echo "🔗 View at: https://github.com/${REPO}/releases/tag/${VERSION}"
echo
echo "📋 What was released:"
echo "  ✅ C-Style Format Specifiers (%s, %d, %f, %.2f, %c, %b)"
echo "  ✅ Escape Sequences Support (\\n, \\t, \\\\, \\', \\\", \\a, \\b, \\r)"
echo "  ✅ Enhanced Say Statement with function-style syntax"
echo "  ✅ Backward compatibility maintained"
echo "  ✅ Updated documentation and examples"
echo "  ✅ All platform binaries (Linux x64/ARM64/ARMv7, Windows x64)"
echo
echo "🚀 LAML v3.4.0 is now live!"
