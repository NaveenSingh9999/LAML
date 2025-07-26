#!/bin/bash

# GitHub Release Publisher for LAML v3.3.0 (Updated)
# This script helps create the GitHub release

set -e

VERSION="v3.3.0-updated"
REPO="NaveenSingh9999/LAML"
RELEASE_TITLE="LAML v3.3.0 (Updated) - Fixed Installation URLs"

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
    echo "4. Description: Copy from RELEASE-NOTES-v3.3.0-updated.md"
    echo "5. Upload assets:"
    echo "   - laml-v3.3.0-updated.tar.gz"
    echo "   - laml-v3.3.0-updated.zip"
    echo "   - laml (binary)"
    echo "   - vscode-extension/laml-3.3.0.vsix"
    echo
    echo "✅ Release files ready in current directory!"
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
gh release create "${VERSION}" \
    --repo "${REPO}" \
    --title "${RELEASE_TITLE}" \
    --notes-file "RELEASE-NOTES-v3.3.0-updated.md" \
    --prerelease \
    "laml-v3.3.0-updated.tar.gz#Complete Release Package (tar.gz)" \
    "laml-v3.3.0-updated.zip#Complete Release Package (zip)" \
    "laml#LAML Universal Binary" \
    "vscode-extension/laml-3.3.0.vsix#VS Code Extension"

echo
echo "🎉 GitHub release created successfully!"
echo "🔗 View at: https://github.com/${REPO}/releases/tag/${VERSION}"
echo
echo "📋 Quick install commands for users:"
echo
echo "Linux/macOS:"
echo "curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash"
echo
echo "Windows (PowerShell as Admin):"
echo "iwr -useb https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1 | iex"
