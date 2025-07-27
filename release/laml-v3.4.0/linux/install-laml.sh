#!/bin/bash
echo "🚀 LAML Linux Installer"
echo "======================="
echo "This will install LAML on your Linux system."
echo ""
echo "Detected architecture: $(uname -m)"
echo ""
read -p "Continue? (y/N): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    bash "$(dirname "$0")/install.sh"
else
    echo "Installation cancelled."
fi
