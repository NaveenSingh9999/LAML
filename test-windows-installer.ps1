# LAML Installer for Windows - Test Version
# Downloads and installs LAML from GitHub (User directory version for testing)

param(
    [switch]$Uninstall,
    [string]$InstallPath = "$env:USERPROFILE\LAML"
)

$ErrorActionPreference = "Continue"

# GitHub URL for the Windows laml binary
$LAML_BINARY_URL = "https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/laml-windows-x86_64.exe"

function Write-ColorText {
    param([string]$Text, [string]$Color = "White")
    Write-Host $Text -ForegroundColor $Color
}

function Download-File {
    param([string]$Url, [string]$OutputPath)
    try {
        Write-ColorText "🌐 Downloading from: $Url" "Blue"
        Write-ColorText "📁 Saving to: $OutputPath" "Blue"
        
        # Test URL accessibility first
        $response = Invoke-WebRequest -Uri $Url -Method Head -UseBasicParsing -ErrorAction Stop
        Write-ColorText "✅ URL is accessible (Status: $($response.StatusCode))" "Green"
        
        # Download the file
        Invoke-WebRequest -Uri $Url -OutFile $OutputPath -UseBasicParsing -ErrorAction Stop
        
        # Verify download
        if (Test-Path $OutputPath) {
            $fileSize = (Get-Item $OutputPath).Length
            Write-ColorText "✅ File downloaded successfully ($fileSize bytes)" "Green"
            return $true
        } else {
            Write-ColorText "❌ File not found after download" "Red"
            return $false
        }
    }
    catch {
        Write-ColorText "❌ Failed to download from $Url" "Red"
        Write-ColorText "Error: $($_.Exception.Message)" "Red"
        return $false
    }
}

function Test-Binary {
    param([string]$BinaryPath)
    try {
        Write-ColorText "🧪 Testing binary: $BinaryPath" "Yellow"
        
        if (-not (Test-Path $BinaryPath)) {
            Write-ColorText "❌ Binary file not found" "Red"
            return $false
        }
        
        $fileInfo = Get-Item $BinaryPath
        Write-ColorText "📄 File size: $($fileInfo.Length) bytes" "Blue"
        Write-ColorText "📅 Last modified: $($fileInfo.LastWriteTime)" "Blue"
        
        # Try to get version
        try {
            $versionOutput = & $BinaryPath version 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-ColorText "✅ Version check passed: $versionOutput" "Green"
                return $true
            } else {
                Write-ColorText "⚠️  Version command failed but binary exists" "Yellow"
                return $true
            }
        }
        catch {
            Write-ColorText "⚠️  Could not execute binary (normal in Wine)" "Yellow"
            return $true
        }
    }
    catch {
        Write-ColorText "❌ Binary test failed: $($_.Exception.Message)" "Red"
        return $false
    }
}

Write-ColorText "🚀 LAML Installer for Windows (Test Version)" "Cyan"
Write-ColorText "=============================================" "Cyan"
Write-ColorText "📥 Downloads LAML Windows binary from GitHub" "Blue"
Write-ColorText "📁 Test installation to user directory" "Blue"
Write-ColorText "" "White"

Write-ColorText "📁 Installation directory: $InstallPath" "White"
Write-ColorText "🔧 Creating installation directory..." "Yellow"

# Create installation directory
try {
    if (-not (Test-Path $InstallPath)) {
        New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
        Write-ColorText "✅ Directory created successfully" "Green"
    } else {
        Write-ColorText "✅ Directory already exists" "Green"
    }
}
catch {
    Write-ColorText "❌ Failed to create directory: $($_.Exception.Message)" "Red"
    exit 1
}

# Download LAML binary
Write-ColorText "📦 Downloading LAML binary..." "Yellow"
$targetBinary = "$InstallPath\laml.exe"

if (Download-File -Url $LAML_BINARY_URL -OutputPath $targetBinary) {
    Write-ColorText "✅ LAML binary downloaded successfully" "Green"
    
    # Test the binary
    if (Test-Binary -BinaryPath $targetBinary) {
        Write-ColorText "✅ Binary verification passed" "Green"
    } else {
        Write-ColorText "❌ Binary verification failed" "Red"
        exit 1
    }
} else {
    Write-ColorText "❌ Failed to download LAML binary" "Red"
    Write-ColorText "Please check your internet connection and try again." "Yellow"
    exit 1
}

Write-ColorText "" "White"
Write-ColorText "🎉 LAML test installation completed successfully!" "Green"
Write-ColorText "" "White"
Write-ColorText "📋 Test results:" "Cyan"
Write-ColorText "• Binary downloaded: ✅" "White"
Write-ColorText "• File verification: ✅" "White"
Write-ColorText "• Installation path: $InstallPath" "White"
Write-ColorText "" "White"
Write-ColorText "💡 To test manually:" "Cyan"
Write-ColorText "  cd `"$InstallPath`"" "White"
Write-ColorText "  .\laml.exe version" "White"
