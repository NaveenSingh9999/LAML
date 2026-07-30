#!/bin/bash
# LAML-NG Release Publisher
# Builds the C++20 NG binary and publishes to GitHub

set -e

VERSION="v4.0.0"
REPO="NaveenSingh9999/LAML"
RELEASE_TITLE="LAML v4.0.0 - C++20 Next-Generation Runtime"

LAMLDIR="$(cd "$(dirname "$0")" && pwd)"
NGDIR="$LAMLDIR/ng"

echo " Building LAML v4.0.0 (C++20 NG) Release"
echo "=========================================="
echo

# Step 1: Build the C++ binary
echo "[1/3] Building C++20 NG binary..."
cd "$NGDIR"
make clean 2>/dev/null || true
make -j4 2>&1
cp laml "$LAMLDIR/laml"
cd "$LAMLDIR"

echo "[2/3] Verifying binary..."
if ! file laml | grep -q ELF; then
    echo " ERROR: Built binary is not valid!"
    exit 1
fi
echo " Binary: $(file laml)"
echo " Size: $(ls -lh laml | awk '{print $5}')"
echo

# Step 2: Create release archive
echo "[3/3] Creating release package..."
tar czf "laml-${VERSION}.tar.gz" \
    laml \
    ng/server.lm \
    ng/www/ \
    ng/examples/ \
    README.md \
    LAML-NG-GUIDE.md

echo " Package: laml-${VERSION}.tar.gz ($(ls -lh laml-${VERSION}.tar.gz | awk '{print $5}'))"
echo

# Step 3: Publish (if gh CLI available)
if command -v gh >/dev/null 2>&1; then
    if gh auth status >/dev/null 2>&1; then
        echo " Publishing to GitHub..."
        gh release create "${VERSION}" \
            --repo "${REPO}" \
            --title "${RELEASE_TITLE}" \
            --notes-file LAML-NG-GUIDE.md \
            "laml#LAML-NG C++20 binary (aarch64)" \
            "laml-${VERSION}.tar.gz#Source + docs archive"
        echo " Published: https://github.com/${REPO}/releases/tag/${VERSION}"
    else
        echo " Run 'gh auth login' first, then re-run this script."
        echo " Assets ready in current directory."
    fi
else
    echo " GitHub CLI not found."
    echo " Assets ready for manual upload:"
    echo "   - laml (binary)"
    echo "   - laml-${VERSION}.tar.gz (archive)"
fi

echo
echo " Done. LAML v4.0.0 C++20 NG release package ready."
