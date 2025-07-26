#!/bin/bash

# LAML v3.3.0 (Updated) Release Packager
# Creates release archives for GitHub

set -e

VERSION="v3.3.0-updated"
RELEASE_DIR="release-${VERSION}"

echo "🚀 Creating LAML ${VERSION} release package..."

# Clean and create release directory
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Copy main binary
echo "📦 Copying main binary..."
cp laml "$RELEASE_DIR/"

# Copy installers
echo "📦 Copying installers..."
mkdir -p "$RELEASE_DIR/installers"
cp -r installers/* "$RELEASE_DIR/installers/"

# Copy documentation
echo "📦 Copying documentation..."
mkdir -p "$RELEASE_DIR/docs"
cp -r docs/* "$RELEASE_DIR/docs/"

# Copy VS Code extension
echo "📦 Copying VS Code extension..."
mkdir -p "$RELEASE_DIR/vscode-extension"
cp vscode-extension/laml-3.3.0.vsix "$RELEASE_DIR/vscode-extension/"

# Copy examples
echo "📦 Copying examples..."
mkdir -p "$RELEASE_DIR/examples"
cp examples/*.lm "$RELEASE_DIR/examples/"

# Copy key files
echo "📦 Copying documentation files..."
cp README.md "$RELEASE_DIR/"
cp INSTALL.md "$RELEASE_DIR/"
cp CHANGELOG-v3.3.0.md "$RELEASE_DIR/"
cp RELEASE-NOTES-v3.3.0-updated.md "$RELEASE_DIR/"

# Create install script for easy setup
cat > "$RELEASE_DIR/quick-install.sh" << 'EOF'
#!/bin/bash

# LAML Quick Install Script
# Choose your platform and run the appropriate installer

echo "🚀 LAML v3.3.0 (Updated) Quick Install"
echo "====================================="
echo
echo "Choose your platform:"
echo "1) Linux"
echo "2) macOS" 
echo "3) Windows (run in PowerShell as Admin)"
echo "4) Termux (Android)"
echo "5) Manual installation"
echo

read -p "Enter choice (1-5): " choice

case $choice in
    1)
        echo "Installing for Linux..."
        curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
        ;;
    2)
        echo "Installing for macOS..."
        curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/macos/install.sh | bash
        ;;
    3)
        echo "Copy and run this command in PowerShell as Administrator:"
        echo "iwr -useb https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1 | iex"
        ;;
    4)
        echo "Installing for Termux..."
        curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/termux/install.sh | bash
        ;;
    5)
        echo "Manual installation:"
        echo "1. Copy the 'laml' binary to your PATH"
        echo "2. Make it executable: chmod +x laml"
        echo "3. Test with: laml version"
        ;;
    *)
        echo "Invalid choice. Please run again and select 1-5."
        ;;
esac
EOF

chmod +x "$RELEASE_DIR/quick-install.sh"

# Create README for release
cat > "$RELEASE_DIR/README-RELEASE.md" << EOF
# LAML v3.3.0 (Updated) Release

This release package contains everything you need to get started with LAML.

## 🚀 Quick Installation

Run the quick install script:
\`\`\`bash
./quick-install.sh
\`\`\`

Or use platform-specific one-liners:

**Linux/macOS:**
\`\`\`bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
\`\`\`

**Windows (PowerShell as Admin):**
\`\`\`powershell
iwr -useb https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1 | iex
\`\`\`

## 📁 Package Contents

- \`laml\` - Main LAML binary (universal)
- \`installers/\` - Platform-specific installation scripts
- \`docs/\` - Complete documentation website
- \`examples/\` - LAML code examples
- \`vscode-extension/\` - VS Code extension for LAML
- \`quick-install.sh\` - Interactive installation script

## 📚 Documentation

- [Learn LAML](docs/learn.html) - Interactive tutorial
- [Setup Guide](docs/setup.html) - Installation instructions  
- [Features](docs/features.html) - Language features overview
- [FAQ](docs/faq.html) - Frequently asked questions

## 🔧 Manual Installation

1. Copy \`laml\` binary to your PATH (e.g., \`/usr/local/bin/\`)
2. Make it executable: \`chmod +x laml\`
3. Verify: \`laml version\`

## 🐛 Support

- GitHub Issues: https://github.com/NaveenSingh9999/LAML/issues
- Documentation: https://naveensingh9999.github.io/LAML/
EOF

# Create archives
echo "📦 Creating release archives..."

# Create tar.gz
tar -czf "laml-${VERSION}.tar.gz" -C . "$RELEASE_DIR"

# Create zip  
zip -r "laml-${VERSION}.zip" "$RELEASE_DIR/" > /dev/null

echo "✅ Release packages created:"
echo "   📦 laml-${VERSION}.tar.gz"
echo "   📦 laml-${VERSION}.zip"
echo "   📁 ${RELEASE_DIR}/"
echo
echo "🚀 Ready for GitHub release!"
echo "   Upload both archives as release assets"
echo "   Use RELEASE-NOTES-v3.3.0-updated.md as release description"
