#!/bin/bash

echo "🔨 Building LAML VS Code Extension..."

cd vscode-extension

# Install vsce if not available
if ! command -v vsce &> /dev/null; then
    echo "📦 Installing vsce (VS Code Extension CLI)..."
    npm install -g vsce
fi

# Copy SVG as PNG placeholder (in real scenario, convert SVG to PNG)
echo "🖼️  Preparing icons..."
cp icons/laml-icon.svg icons/laml-icon.png

# Package extension
echo "📦 Packaging extension..."
vsce package --no-yarn

if [ $? -eq 0 ]; then
    echo "✅ Extension packaged successfully!"
    echo "📁 VSIX file created: $(ls *.vsix)"
    mv *.vsix ../installers/
else
    echo "❌ Extension packaging failed"
    exit 1
fi

cd ..
echo "🎉 VS Code extension ready for installation!"
