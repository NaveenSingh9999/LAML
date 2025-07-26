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
