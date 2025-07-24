#!/data/data/com.termux/files/usr/bin/bash
echo "🚀 LAML Termux Installer"
echo "========================"
echo "Installing LAML for Android/Termux..."
echo ""

# Download and run installer
if command -v curl >/dev/null 2>&1; then
    bash "$(dirname "$0")/install.sh"
else
    echo "Installing curl first..."
    pkg install -y curl
    bash "$(dirname "$0")/install.sh"
fi
