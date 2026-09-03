#!/bin/bash
# LAML v4.1.0 Release Publisher
# Builds the C++20 binary and publishes to GitHub

set -e

VERSION="v4.1.0"
REPO="NaveenSingh9999/LAML"
RELEASE_TITLE="LAML v4.1.0 - Realtime Language"
ARCH="$(uname -m)"

LAMLDIR="$(cd "$(dirname "$0")" && pwd)"
NGDIR="$LAMLDIR/ng"

echo " Building LAML v4.1.0 release"
echo "==============================="
echo

# Step 1: Build the C++ binary
echo "[1/4] Building C++20 binary..."
cd "$NGDIR"
make clean 2>/dev/null || true
make -j4 2>&1 | grep -E "error|warning" || true
cp laml "$LAMLDIR/laml"
cp laml "$LAMLDIR/laml-linux-${ARCH}"
cd "$LAMLDIR"

echo "[2/4] Verifying binary..."
if ! file laml | grep -q ELF; then
    echo " ERROR: Built binary is not valid!"
    exit 1
fi
./laml version
echo " Binary: $(file laml)"
echo " Size: $(ls -lh laml | awk '{print $5}')"
echo

# Step 2: Smoke tests
echo "[3/4] Smoke tests..."
./laml run ng/examples/hello.lm
./laml run ng/examples/test_general.lm | tail -1
echo

# Step 3: Create release archive
echo "[4/4] Creating release package..."
tar czf "laml-${VERSION}.tar.gz" \
    "laml-linux-${ARCH}" \
    ng/server.lm \
    ng/www/ \
    ng/examples/ \
    ng/BENCH.md \
    README.md \
    LAML-NG-GUIDE.md \
    LAML-v4.1-REALTIME-DESIGN.md

echo " Package: laml-${VERSION}.tar.gz ($(ls -lh laml-${VERSION}.tar.gz | awk '{print $5}'))"
echo

# Step 4: Publish (if gh CLI available)
NOTES="$(mktemp)"
cat > "$NOTES" <<EOF
LAML v4.1.0 — a dynamic language for writing realtime servers.

Highlights: WebSocket + HTTP/1.1 + SSE on one port (serve/on/broadcast
rooms, presence, timers, JSON), general-purpose stdlib (math, arrays,
strings, objects, files, assert), float/mixed comparisons, closc
priorities, graceful drain. Measured: 7350/7350 WS fan-out delivered,
2000 idle WS at 13MB. Full numbers in ng/BENCH.md.

Install: \`curl -fsSL https://github.com/NaveenSingh9999/LAML/raw/refs/heads/main/install.sh | bash\`
EOF
if command -v gh >/dev/null 2>&1; then
    if gh auth status >/dev/null 2>&1; then
        echo " Publishing to GitHub..."
        gh release create "${VERSION}" \
            --repo "${REPO}" \
            --title "${RELEASE_TITLE}" \
            --notes-file "$NOTES" \
            "laml-linux-${ARCH}#Prebuilt binary (linux-${ARCH})" \
            "laml-${VERSION}.tar.gz#Binary + examples + docs"
        echo " Published: https://github.com/${REPO}/releases/tag/${VERSION}"
    else
        echo " Run 'gh auth login' first, then re-run this script."
        echo " Assets ready in current directory."
    fi
else
    echo " GitHub CLI not found."
    echo " Assets ready for manual upload:"
    echo "   - laml-linux-${ARCH} (binary)"
    echo "   - laml-${VERSION}.tar.gz (archive)"
fi
rm -f "$NOTES"

echo
echo " Done. LAML v4.1.0 release package ready."
