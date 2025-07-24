# LAML for Termux (Android)

## Installation

1. Open Termux
2. Install dependencies:
   ```bash
   pkg update && pkg install curl unzip
   ```

3. Download and run installer:
   ```bash
   curl -sSL https://github.com/NaveenSingh9999/LAML/releases/latest/download/install-termux.sh | bash
   ```

## Manual Installation

1. Make the installer executable:
   ```bash
   chmod +x install-termux.sh
   ```

2. Run the installer:
   ```bash
   ./install-termux.sh
   ```

## Features

- Native ARM/ARM64 compilation
- Android storage integration
- Termux widget shortcuts
- Development environment setup

## Requirements

- Termux app from F-Droid or Google Play
- Android 5.0+ (API level 21+)
- Storage permission (granted during installation)

## Quick Start

After installation:
```bash
cd ~/laml-examples
laml run hello-termux.lm
```

For support, visit: https://github.com/NaveenSingh9999/LAML
