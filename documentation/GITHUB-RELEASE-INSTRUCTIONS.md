# 🚀 Manual GitHub Release Instructions

## Release Information
- **Version**: v3.3.0-installer-fix
- **Title**: LAML v3.3.0 (Updated) - Fixed Installation URLs  
- **Type**: Pre-release (updated/unofficial)

## 📋 Step-by-Step Release Process

### 1. Go to GitHub Releases
Visit: https://github.com/NaveenSingh9999/LAML/releases/new

### 2. Tag Information
- **Tag**: `v3.3.0-installer-fix`
- **Target**: `main` branch

### 3. Release Title
```
LAML v3.3.0 (Updated) - Fixed Installation URLs
```

### 4. Release Description
Copy the entire content from: `RELEASE-NOTES-v3.3.0-updated.md`

### 5. Upload Assets
Upload these files as release assets:

- **laml-v3.3.0-updated.tar.gz** (Complete release package)
- **laml-v3.3.0-updated.zip** (Complete release package)  
- **laml** (Universal binary)
- **vscode-extension/laml-3.3.0.vsix** (VS Code extension)

### 6. Release Options
- ✅ **Check "This is a pre-release"** (since it's an updated/unofficial release)
- ✅ **Check "Create a discussion for this release"** (optional)

### 7. Publish
Click **"Publish release"**

## 📦 Available Files

All release files are ready in the current directory:

```bash
# Release packages (created by create-release.sh)
laml-v3.3.0-updated.tar.gz  # Complete package
laml-v3.3.0-updated.zip     # Complete package

# Individual assets  
laml                        # Universal binary
vscode-extension/laml-3.3.0.vsix  # VS Code extension
RELEASE-NOTES-v3.3.0-updated.md   # Release description
```

## 🔧 Quick Installation Commands

After publishing, users can install with:

**Linux/macOS:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/linux/install.sh | bash
```

**Windows (PowerShell as Admin):**
```powershell
iwr -useb https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/windows/install.ps1 | iex
```

**Termux:**
```bash
curl -fsSL https://raw.githubusercontent.com/NaveenSingh9999/LAML/refs/heads/main/installers/termux/install.sh | bash
```

## ✅ Verification

After release, test the installers work:
```bash
# Test version command
laml version

# Test hello world
echo 'bring xcs.class34; func main() { say "Hello, LAML v3.3.0!"; }' > test.lm
laml run test.lm
```

---

**Ready to publish!** 🎉
