# LAML VS Code Extension

## Installation

### Method 1: VSIX Package (Recommended)
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Click the "..." menu and select "Install from VSIX..."
4. Select the `laml-3.2.0.vsix` file

### Method 2: Manual Installation
1. Copy the extension folder to your VS Code extensions directory:
   - Windows: `%USERPROFILE%\.vscode\extensions\`
   - macOS: `~/.vscode/extensions/`
   - Linux: `~/.vscode/extensions/`

## Features

- Syntax highlighting for .lm files
- Comment support (~ and {~ ~})
- Auto-completion
- File icons
- Language configuration

## File Association

The extension automatically associates `.lm` files with LAML syntax highlighting.

## Development

To contribute to the extension:
1. Install Node.js and npm
2. Install vsce: `npm install -g vsce`
3. Make changes to the extension
4. Package: `vsce package`

For support, visit: https://github.com/NaveenSingh9999/LAML
