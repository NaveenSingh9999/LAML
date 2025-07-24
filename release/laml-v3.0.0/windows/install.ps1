# LAML Installer for Windows
# PowerShell script for easy installation

param(
    [switch]$Uninstall,
    [string]$InstallPath = "$env:ProgramFiles\LAML"
)

$ErrorActionPreference = "Stop"

function Write-ColorText {
    param([string]$Text, [string]$Color = "White")
    Write-Host $Text -ForegroundColor $Color
}

function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if ($Uninstall) {
    Write-ColorText "🗑️  Uninstalling LAML..." "Yellow"
    
    if (Test-Path $InstallPath) {
        Remove-Item -Path $InstallPath -Recurse -Force
        Write-ColorText "✅ LAML removed from $InstallPath" "Green"
    }
    
    # Remove from PATH
    $envPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
    if ($envPath -like "*$InstallPath*") {
        $newPath = $envPath.Replace(";$InstallPath", "").Replace("$InstallPath;", "").Replace("$InstallPath", "")
        [Environment]::SetEnvironmentVariable("PATH", $newPath, "Machine")
        Write-ColorText "✅ Removed from system PATH" "Green"
    }
    
    Write-ColorText "🎉 LAML uninstalled successfully!" "Green"
    exit 0
}

Write-ColorText "🚀 LAML Installer for Windows" "Cyan"
Write-ColorText "==============================" "Cyan"

if (-not (Test-Administrator)) {
    Write-ColorText "❌ This installer requires administrator privileges." "Red"
    Write-ColorText "Please run PowerShell as Administrator and try again." "Yellow"
    exit 1
}

Write-ColorText "📁 Installation directory: $InstallPath" "White"
Write-ColorText "🔧 Creating installation directory..." "Yellow"

# Create installation directory
if (-not (Test-Path $InstallPath)) {
    New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
}

# Download or copy LAML binary (placeholder - in real scenario, download from GitHub releases)
Write-ColorText "📦 Installing LAML compiler..." "Yellow"

# For demo purposes, we'll create a placeholder
@"
@echo off
echo LAML Compiler v3.0.0 - Windows
echo This is a placeholder. In production, this would be the actual LAML binary.
echo Usage: laml [command] [file.lm]
echo Commands:
echo   run [file]     - Compile and run LAML file
echo   compile [file] - Compile LAML file to binary
echo   version        - Show version information
"@ | Out-File -FilePath "$InstallPath\laml.bat" -Encoding ASCII

# Add to PATH
Write-ColorText "🔗 Adding LAML to system PATH..." "Yellow"
$envPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
if ($envPath -notlike "*$InstallPath*") {
    [Environment]::SetEnvironmentVariable("PATH", "$envPath;$InstallPath", "Machine")
}

# Create desktop shortcut
Write-ColorText "🖼️  Creating desktop shortcut..." "Yellow"
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$env:USERPROFILE\Desktop\LAML Terminal.lnk")
$Shortcut.TargetPath = "cmd.exe"
$Shortcut.Arguments = "/k echo LAML Development Environment && echo Type 'laml --help' for usage"
$Shortcut.WorkingDirectory = "$env:USERPROFILE"
$Shortcut.IconLocation = "$InstallPath\laml.ico"
$Shortcut.Save()

# Install VS Code extension if VS Code is available
if (Get-Command code -ErrorAction SilentlyContinue) {
    Write-ColorText "📝 Installing VS Code extension..." "Yellow"
    if (Test-Path "$PSScriptRoot\..\laml-*.vsix") {
        $vsixFile = Get-ChildItem "$PSScriptRoot\..\laml-*.vsix" | Select-Object -First 1
        & code --install-extension $vsixFile.FullName
        Write-ColorText "✅ VS Code extension installed" "Green"
    }
}

Write-ColorText "" "White"
Write-ColorText "🎉 LAML installation completed successfully!" "Green"
Write-ColorText "" "White"
Write-ColorText "📋 Next steps:" "Cyan"
Write-ColorText "1. Restart your command prompt or PowerShell" "White"
Write-ColorText "2. Type 'laml version' to verify installation" "White"
Write-ColorText "3. Create your first .lm file and run with 'laml run file.lm'" "White"
Write-ColorText "" "White"
Write-ColorText "📚 Documentation: https://github.com/NaveenSingh9999/LAML" "Blue"
Write-ColorText "🐛 Issues: https://github.com/NaveenSingh9999/LAML/issues" "Blue"
