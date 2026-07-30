#!/usr/bin/env bash
set -euo pipefail

LAML_VERSION="4.0.0"
LAML_REPO="https://github.com/NaveenSingh9999/LAML.git"
LAML_BRANCH="main"
WORK_DIR="${TMPDIR:-/tmp}/laml-install-$$"
INSTALL_DIR="/usr/local/bin"
BUILD_DIR=""

# ---------------------------------------------------------------------------
# Color / UI helpers
# ---------------------------------------------------------------------------
RESET="\033[0m"
BOLD="\033[1m"
DIM="\033[2m"
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
MAGENTA="\033[35m"
CYAN="\033[36m"
WHITE="\033[97m"

ui_init() {
  if [[ ! -t 1 ]]; then
    RESET= BOLD= DIM= RED= GREEN= YELLOW= BLUE= MAGENTA= CYAN= WHITE=
  fi
  cols="${COLUMNS:-$(tput cols 2>/dev/null || echo 80)}"
}

log()  { echo -e "  ${BOLD}${*}${RESET}"; }
ok()   { echo -e "  ${GREEN}${BOLD}OK${RESET}  ${*}"; }
info() { echo -e "  ${CYAN}${BOLD}::${RESET} ${*}"; }
warn() { echo -e "  ${YELLOW}${BOLD}WARN${RESET} ${*}"; }
fail() { echo -e "  ${RED}${BOLD}FAIL${RESET} ${*}"; exit 1; }

blank()     { echo; }
hr()        { printf "  ${DIM}%${cols}s${RESET}\n" "" | tr ' ' '─'; }
title()     { echo -e "\n${BOLD}${WHITE}  ${*}${RESET}\n"; }
subtitle()  { echo -e "  ${BOLD}${CYAN}▸ ${*}${RESET}"; }

# ---------------------------------------------------------------------------
# Spinner (async)
# ---------------------------------------------------------------------------
_spin_pid=""
_spin_chars=('|' '/' '-' '\')

spinner_start() {
  local msg="${1:-Working...}"
  if [[ ! -t 1 ]]; then
    echo "  ${msg}..."
    return
  fi
  printf "  ${DIM}${msg}${RESET}  "
  (
    while true; do
      for c in "${_spin_chars[@]}"; do
        printf "\b%s" "$c"
        sleep 0.12
      done
    done
  ) &
  _spin_pid=$!
  disown
}

spinner_stop() {
  if [[ -n "$_spin_pid" ]]; then
    kill "$_spin_pid" 2>/dev/null || true
    wait "$_spin_pid" 2>/dev/null || true
    _spin_pid=""
    printf "\b \b"
  fi
}

# ---------------------------------------------------------------------------
# Progress bar (determinate)
# ---------------------------------------------------------------------------
progress_bar() {
  local current="$1" total="$2" label="${3:-}"
  if [[ ! -t 1 ]]; then
    [[ -n "$label" ]] && echo "  ${label}: ${current}/${total}"
    return
  fi
  local pct=0
  (( total > 0 )) && pct=$(( current * 100 / total ))
  (( pct > 100 )) && pct=100

  local bar_w=$(( cols - 20 ))
  (( bar_w < 20 )) && bar_w=20
  local fill_w=$(( pct * bar_w / 100 ))
  local empty_w=$(( bar_w - fill_w ))

  local bar="${GREEN}$(printf '█%.0s' $(seq 1 $fill_w))${RESET}"
  bar+="${DIM}$(printf '░%.0s' $(seq 1 $empty_w))${RESET}"

  printf "\r  ${bar} ${BOLD}%3d%%${RESET}  ${DIM}%s${RESET}" "$pct" "$label"
  [[ $pct -eq 100 ]] && echo
}

# ---------------------------------------------------------------------------
# Header / footer
# ---------------------------------------------------------------------------
print_banner() {
  echo -e "${CYAN}${BOLD}"
  echo '  ╔══════════════════════════════════════════════╗'
  echo '  ║        LAML  ⚡  v'"${LAML_VERSION}"'               ║'
  echo '  ║  Low Abstraction Machine Language            ║'
  echo '  ║  Universal Installer                         ║'
  echo '  ╚══════════════════════════════════════════════╝'
  echo -e "${RESET}"
}

print_footer() {
  echo
  hr
  echo -e "  ${GREEN}${BOLD}LAML v${LAML_VERSION} installed successfully!${RESET}"
  echo -e "  ${CYAN}Run ${BOLD}laml version${RESET}${CYAN} to verify.${RESET}"
  echo -e "  ${CYAN}Docs: ${BOLD}https://github.com/NaveenSingh9999/LAML${RESET}"
  echo
}

# ---------------------------------------------------------------------------
# Platform detection
# ---------------------------------------------------------------------------
detect_platform() {
  subtitle "Detecting platform"
  spinner_start "Probing system"

  OS=""
  ARCH=""
  PKG_MGR=""

  case "$(uname -s)" in
    Linux)
      if [[ -n "${PREFIX:-}" && -d "$PREFIX" ]]; then
        OS="termux"
      elif grep -qi android /proc/1/cgroup 2>/dev/null || [[ -f /system/build.prop ]]; then
        OS="termux"
      else
        OS="linux"
      fi
      ;;
    Darwin) OS="macos" ;;
    FreeBSD) OS="freebsd" ;;
    CYGWIN*|MINGW*|MSYS*) OS="windows" ;;
    *) OS="unknown" ;;
  esac

  case "$(uname -m)" in
    x86_64|amd64)     ARCH="x86_64" ;;
    aarch64|arm64)    ARCH="aarch64" ;;
    armv7l|armv7|arm) ARCH="armv7" ;;
    i686|i386)        ARCH="i686" ;;
    riscv64)          ARCH="riscv64" ;;
    *)                ARCH="unknown" ;;
  esac

  case "$OS" in
    linux)
      if command -v apt &>/dev/null; then
        PKG_MGR="apt"
      elif command -v dnf &>/dev/null; then
        PKG_MGR="dnf"
      elif command -v yum &>/dev/null; then
        PKG_MGR="yum"
      elif command -v pacman &>/dev/null; then
        PKG_MGR="pacman"
      elif command -v zypper &>/dev/null; then
        PKG_MGR="zypper"
      elif command -v apk &>/dev/null; then
        PKG_MGR="apk"
      fi
      ;;
    macos)
      command -v brew &>/dev/null && PKG_MGR="brew"
      ;;
    termux)
      command -v pkg &>/dev/null && PKG_MGR="pkg"
      ;;
  esac

  spinner_stop
  echo -e "  ${DIM}OS:${RESET} ${BOLD}$OS${RESET}  ${DIM}Arch:${RESET} ${BOLD}$ARCH${RESET}  ${DIM}Packager:${RESET} ${BOLD}${PKG_MGR:-(none)}${RESET}"
}

# ---------------------------------------------------------------------------
# Dependency checks
# ---------------------------------------------------------------------------
check_deps() {
  subtitle "Checking build dependencies"
  local missing=()

  # clang++ or g++
  if command -v clang++ &>/dev/null; then
    CXX=clang++
    ok "clang++ found: $(clang++ --version | head -1)"
  elif command -v g++ &>/dev/null; then
    CXX=g++
    warn "clang++ not found, falling back to g++"
  else
    missing+=("clang++ (or g++)")
  fi

  # llvm-config
  LLVM_CFG=""
  for probe in llvm-config llvm-config-21 llvm-config-20 llvm-config-19 llvm-config-18 llvm-config-17 llvm-config-16 llvm-config-15 llvm-config-14; do
    if command -v "$probe" &>/dev/null; then
      LLVM_CFG="$probe"
      break
    fi
  done

  if [[ -n "$LLVM_CFG" ]]; then
    local lv
    lv="$($LLVM_CFG --version 2>/dev/null || echo "?")"
    ok "llvm-config found: $LLVM_CFG (LLVM $lv)"
  else
    missing+=("llvm-config (install llvm-dev / llvm-devel)")
  fi

  command -v make &>/dev/null || missing+=("make")
  command -v git &>/dev/null || missing+=("git")
  command -v curl &>/dev/null && command -v wget &>/dev/null ||:

  if [[ ${#missing[@]} -gt 0 ]]; then
    blank
    warn "Missing dependencies:"
    for m in "${missing[@]}"; do echo "    - $m"; done
    blank
    install_deps "${missing[@]}"
  else
    ok "All build dependencies satisfied"
  fi
}

# ---------------------------------------------------------------------------
# Auto-install dependencies
# ---------------------------------------------------------------------------
install_deps() {
  info "Attempting to install missing dependencies..."
  blank

  case "$OS" in
    linux)
      case "$PKG_MGR" in
        apt)
          sudo apt-get update -qq
          sudo apt-get install -y -qq clang llvm-dev libllvm21 make git curl 2>&1 | while read -r line; do
            echo "  ${DIM}apt:${RESET} $line"
          done
          ;;
        dnf)
          sudo dnf install -y clang llvm-devel make git curl 2>&1 | while read -r line; do
            echo "  ${DIM}dnf:${RESET} $line"
          done
          ;;
        pacman)
          sudo pacman -S --noconfirm clang llvm make git curl 2>&1 | while read -r line; do
            echo "  ${DIM}pacman:${RESET} $line"
          done
          ;;
        zypper)
          sudo zypper --non-interactive install clang llvm-devel make git curl 2>&1 | while read -r line; do
            echo "  ${DIM}zypper:${RESET} $line"
          done
          ;;
        apk)
          sudo apk add clang llvm-dev make git curl 2>&1 | while read -r line; do
            echo "  ${DIM}apk:${RESET} $line"
          done
          ;;
        *)
          fail "Auto-install not supported for this distro. Install manually:\n    clang++ llvm-dev make git"
          ;;
      esac
      ;;
    macos)
      if [[ "$PKG_MGR" == "brew" ]]; then
        brew install llvm make git curl 2>&1 | while read -r line; do
          echo "  ${DIM}brew:${RESET} $line"
        done
        # Homebrew llvm is keg-only — add to PATH for this session
        if [[ -z "$LLVM_CFG" ]]; then
          for prefix in /usr/local/opt/llvm /opt/homebrew/opt/llvm; do
            if [[ -x "$prefix/bin/llvm-config" ]]; then
              export PATH="$prefix/bin:$PATH"
              LLVM_CFG="$prefix/bin/llvm-config"
              break
            fi
          done
        fi
      else
        fail "Homebrew not found. Install it or install dependencies manually."
      fi
      ;;
    termux)
      pkg update -y 2>&1 | tail -1
      pkg install -y clang llvm make git curl 2>&1 | while read -r line; do
        echo "  ${DIM}pkg:${RESET} $line"
      done
      ;;
    freebsd)
      sudo pkg install -y llvm make git curl 2>&1 | while read -r line; do
        echo "  ${DIM}pkg:${RESET} $line"
      done
      ;;
    *)
      fail "No package manager known for $OS. Install deps manually:\n    clang++ llvm-dev make git"
      ;;
  esac

  blank
  info "Verifying dependencies after install..."
  if ! command -v clang++ &>/dev/null && command -v g++ &>/dev/null; then
    CXX=g++
    warn "Using g++ as C++ compiler"
  elif command -v clang++ &>/dev/null; then
    CXX=clang++
  else
    fail "C++ compiler still not found after install"
  fi
  if [[ -z "$LLVM_CFG" ]]; then
    for probe in llvm-config llvm-config-21 llvm-config-20 llvm-config-19; do
      command -v "$probe" &>/dev/null && { LLVM_CFG="$probe"; break; }
    done
  fi
  if [[ -z "$LLVM_CFG" ]]; then
    fail "llvm-config still not found after install"
  fi
  ok "Dependencies ready"
}

# ---------------------------------------------------------------------------
# Clone / update source
# ---------------------------------------------------------------------------
clone_source() {
  subtitle "Retrieving LAML source"

  if [[ -d "$WORK_DIR" ]]; then
    rm -rf "$WORK_DIR"
  fi
  mkdir -p "$WORK_DIR"

  local clone_start
  clone_start=$(date +%s)

  if git clone --depth 1 --branch "$LAML_BRANCH" "$LAML_REPO" "$WORK_DIR/src" 2>&1; then
    local clone_end
    clone_end=$(date +%s)
    ok "Source cloned from GitHub ($(( clone_end - clone_start ))s)"
  else
    fail "Failed to clone repository from $LAML_REPO"
  fi

  BUILD_DIR="$WORK_DIR/src/ng"
  if [[ ! -d "$BUILD_DIR" ]]; then
    BUILD_DIR="$WORK_DIR/src"
    if [[ ! -f "$BUILD_DIR/Makefile" ]]; then
      fail "Could not find build directory (ng/) in cloned repo"
    fi
  fi
}

# ---------------------------------------------------------------------------
# Build from source
# ---------------------------------------------------------------------------
build_source() {
  subtitle "Compiling LAML from source"
  blank

  local cores
  cores="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)"

  export CXX="${CXX:-clang++}"
  export LLVM_CFG="${LLVM_CFG:-llvm-config}"

  echo -e "  ${DIM}Compiler:${RESET} ${BOLD}$CXX${RESET}  ${DIM}Jobs:${RESET} ${BOLD}$cores${RESET}  ${DIM}LLVM:${RESET} ${BOLD}$($LLVM_CFG --version 2>/dev/null || echo "?")${RESET}"
  blank

  cd "$BUILD_DIR"

  # Clean first
  make clean 2>/dev/null || true

  # Build with progress (approximate: parse make output for link step)
  local build_ok=0
  make -j"$cores" 2>&1 | while IFS= read -r line; do
    if [[ "$line" == *"error:"* ]]; then
      echo -e "  ${RED}${line}${RESET}"
    elif [[ "$line" == *"warning:"* ]]; then
      echo -e "  ${YELLOW}${line}${RESET}"
    elif [[ "$line" == *"clang++"* || "$line" == *"g++"* ]]; then
      local obj
      obj="$(echo "$line" | grep -oP '\b\w+\.o\b' | tail -1)"
      echo -e "  ${DIM}CC${RESET}  ${obj:-compile}"
    fi
  done

  if [[ -f "$BUILD_DIR/laml" ]]; then
    ok "Build complete"
  else
    fail "Binary not produced — check build output above"
  fi

  # Measure binary
  local size
  size=$(stat -c%s "$BUILD_DIR/laml" 2>/dev/null || stat -f%z "$BUILD_DIR/laml" 2>/dev/null || echo 0)
  echo -e "  ${DIM}Binary size:${RESET} ${BOLD}$(numfmt --to=iec $size 2>/dev/null || echo "${size}B")${RESET}"
}

# ---------------------------------------------------------------------------
# Install binary
# ---------------------------------------------------------------------------
install_binary() {
  subtitle "Installing LAML binary"

  local src="$BUILD_DIR/laml"

  if [[ "$OS" == "termux" ]]; then
    cp "$src" "$PREFIX/bin/laml"
    chmod +x "$PREFIX/bin/laml"
    ok "Installed to $PREFIX/bin/laml"
  elif [[ "$OS" == "macos" || "$OS" == "linux" || "$OS" == "freebsd" ]]; then
    if command -v sudo &>/dev/null; then
      sudo cp "$src" "$INSTALL_DIR/laml"
      sudo chmod +x "$INSTALL_DIR/laml"
    else
      cp "$src" "$INSTALL_DIR/laml"
      chmod +x "$INSTALL_DIR/laml"
    fi
    ok "Installed to $INSTALL_DIR/laml"
  else
    cp "$src" "$INSTALL_DIR/laml"
    chmod +x "$INSTALL_DIR/laml"
    ok "Installed to $INSTALL_DIR/laml"
  fi
}

# ---------------------------------------------------------------------------
# Verify
# ---------------------------------------------------------------------------
verify() {
  subtitle "Verifying installation"
  if command -v laml &>/dev/null; then
    local ver
    ver="$(laml version 2>&1 || true)"
    echo -e "  ${GREEN}✓${RESET} laml found in PATH"
    echo -e "  ${DIM}  Version:${RESET} ${BOLD}$ver${RESET}"
  else
    warn "laml not in PATH"
    case "$OS" in
      termux) echo "  ${CYAN}  Add to PATH: export PATH=\$PATH:$PREFIX/bin${RESET}" ;;
      *)      echo "  ${CYAN}  Add to PATH: export PATH=\$PATH:$INSTALL_DIR${RESET}" ;;
    esac
  fi

  local test_file="$WORK_DIR/.laml_verify.lm"
  echo 'say "LAML installation verified!";' > "$test_file"
  if laml "$test_file" &>/dev/null; then
    ok "Test program executed successfully"
  else
    warn "Test program execution failed"
  fi
  rm -f "$test_file"
}

# ---------------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------------
cleanup() {
  if [[ -d "$WORK_DIR" ]]; then
    rm -rf "$WORK_DIR"
  fi
}

trap cleanup EXIT INT TERM

# ===========================================================================
# Main
# ===========================================================================
main() {
  ui_init
  print_banner

  hr
  detect_platform
  hr
  check_deps
  hr
  clone_source
  hr
  build_source
  hr
  install_binary
  hr
  verify
  hr
  print_footer
}

main "$@"
