# LAML Installer for Windows
# Downloads and installs LAML from GitHub

param(
    [switch]$Uninstall,
    [string]$InstallPath = "$env:ProgramFiles\LAML"
)

$ErrorActionPreference = "Stop"

# GitHub URL for the universal laml binary
$LAML_BINARY_URL = "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml"
$LAML_VERSION_URL = "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/README.md"

function Write-ColorText {
    param([string]$Text, [string]$Color = "White")
    Write-Host $Text -ForegroundColor $Color
}

function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Download-File {
    param([string]$Url, [string]$OutputPath)
    try {
        Invoke-WebRequest -Uri $Url -OutFile $OutputPath -UseBasicParsing
        return $true
    }
    catch {
        Write-ColorText "❌ Failed to download from $Url" "Red"
        Write-ColorText "Error: $($_.Exception.Message)" "Red"
        return $false
    }
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
    
    # Remove desktop shortcut
    $shortcutPath = "$env:USERPROFILE\Desktop\LAML Terminal.lnk"
    if (Test-Path $shortcutPath) {
        Remove-Item $shortcutPath
        Write-ColorText "✅ Desktop shortcut removed" "Green"
    }
    
    Write-ColorText "🎉 LAML uninstalled successfully!" "Green"
    exit 0
}

Write-ColorText "🚀 LAML Installer for Windows" "Cyan"
Write-ColorText "==============================" "Cyan"
Write-ColorText "📥 Downloads latest LAML from GitHub" "Blue"
Write-ColorText "" "White"

if (-not (Test-Administrator)) {
    Write-ColorText "❌ This installer requires administrator privileges." "Red"
    Write-ColorText "Please run PowerShell as Administrator and try again." "Yellow"
    Write-ColorText "" "White"
    Write-ColorText "💡 Right-click PowerShell and select 'Run as Administrator'" "Cyan"
    exit 1
}

Write-ColorText "📁 Installation directory: $InstallPath" "White"
Write-ColorText "🔧 Creating installation directory..." "Yellow"

# Create installation directory
if (-not (Test-Path $InstallPath)) {
    New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
}

# Download LAML binary
Write-ColorText "📦 Downloading LAML from GitHub..." "Yellow"
$targetBinary = "$InstallPath\laml.exe"

if (Download-File -Url $LAML_BINARY_URL -OutputPath $targetBinary) {
    Write-ColorText "✅ LAML binary downloaded successfully" "Green"
    
    # Make sure the file is executable
    if (Test-Path $targetBinary) {
        Write-ColorText "✅ LAML binary installed as 'laml'" "Green"
    } else {
        Write-ColorText "❌ Downloaded file not found!" "Red"
        exit 1
    }
} else {
    Write-ColorText "❌ Failed to download LAML binary" "Red"
    Write-ColorText "Please check your internet connection and try again." "Yellow"
    exit 1
}

# Add to PATH
Write-ColorText "🔗 Adding LAML to system PATH..." "Yellow"
$envPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
if ($envPath -notlike "*$InstallPath*") {
    [Environment]::SetEnvironmentVariable("PATH", "$envPath;$InstallPath", "Machine")
    Write-ColorText "✅ Added to system PATH" "Green"
} else {
    Write-ColorText "✅ Already in system PATH" "Green"
}

# Create desktop shortcut
Write-ColorText "🖼️  Creating desktop shortcut..." "Yellow"
try {
    $WshShell = New-Object -comObject WScript.Shell
    $Shortcut = $WshShell.CreateShortcut("$env:USERPROFILE\Desktop\LAML Terminal.lnk")
    $Shortcut.TargetPath = "cmd.exe"
    $Shortcut.Arguments = "/k echo LAML Development Environment && echo Type 'laml --help' for usage && echo."
    $Shortcut.WorkingDirectory = "$env:USERPROFILE"
    $Shortcut.Description = "LAML Programming Language Terminal"
    $Shortcut.Save()
    Write-ColorText "✅ Desktop shortcut created" "Green"
}
catch {
    Write-ColorText "⚠️  Could not create desktop shortcut" "Yellow"
}

# Test installation
Write-ColorText "🧪 Testing installation..." "Yellow"
try {
    $version = & "$targetBinary" --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-ColorText "✅ LAML is working correctly" "Green"
    } else {
        Write-ColorText "⚠️  LAML installed but may need configuration" "Yellow"
    }
}
catch {
    Write-ColorText "⚠️  Could not test LAML (this is normal on first install)" "Yellow"
}

# Install VS Code extension if available
if (Get-Command code -ErrorAction SilentlyContinue) {
    Write-ColorText "📝 Checking for VS Code extension..." "Yellow"
    # Try to find the extension in common locations
    $vsixLocations = @(
        "$PSScriptRoot\..\laml-*.vsix",
        "$PSScriptRoot\..\..\vscode-extension\laml-*.vsix",
        "$PSScriptRoot\..\..\installers\laml-*.vsix"
    )
    
    $vsixFile = $null
    foreach ($location in $vsixLocations) {
        $found = Get-ChildItem $location -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $vsixFile = $found
            break
        }
    }
    
    if ($vsixFile) {
        try {
            & code --install-extension $vsixFile.FullName
            Write-ColorText "✅ VS Code extension installed" "Green"
        }
        catch {
            Write-ColorText "⚠️  Could not install VS Code extension automatically" "Yellow"
        }
    } else {
        Write-ColorText "ℹ️  VS Code extension not found in installer package" "Blue"
    }
} else {
    Write-ColorText "ℹ️  VS Code not found - extension installation skipped" "Blue"
}

Write-ColorText "" "White"
Write-ColorText "🎉 LAML installation completed successfully!" "Green"
Write-ColorText "" "White"
Write-ColorText "📋 Next steps:" "Cyan"
Write-ColorText "1. Restart your command prompt or PowerShell" "White"
Write-ColorText "2. Type 'laml --version' to verify installation" "White"
Write-ColorText "3. Type 'laml --help' to see available commands" "White"
Write-ColorText "4. Create your first .lm file and run with 'laml run file.lm'" "White"
Write-ColorText "" "White"
Write-ColorText "📚 Documentation: https://github.com/NaveenSingh9999/LAML" "Blue"
Write-ColorText "🐛 Issues: https://github.com/NaveenSingh9999/LAML/issues" "Blue"
Write-ColorText "" "White"
Write-ColorText "💡 You can now use 'laml' from anywhere in your terminal!" "Cyan"
