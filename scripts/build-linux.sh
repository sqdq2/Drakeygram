#!/usr/bin/env bash
# Drakeygram — production Linux build (all-in-one)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

red()    { printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green()  { printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow() { printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die()    { red "$*"; exit 1; }
trap 'die "Build failed at line $LINENO (exit $?)"' ERR

green "=== Drakeygram Linux production build ==="
yellow "Repo: $ROOT"

# --- Credentials (only prompts required) ---
if [[ -z "${TELEGRAM_API_ID:-}" ]]; then
  read -r -p "Telegram API ID: " TELEGRAM_API_ID || true
fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then
  read -r -p "Telegram API Hash: " TELEGRAM_API_HASH || true
fi
[[ "${TELEGRAM_API_ID:-}" =~ ^[0-9]+$ ]] || die "API ID must be numeric — get both from https://my.telegram.org"
[[ -n "${TELEGRAM_API_HASH:-}" ]] || die "API Hash is required — https://my.telegram.org"
export TELEGRAM_API_ID TELEGRAM_API_HASH

# --- Tools ---
need() { command -v "$1" >/dev/null 2>&1 || die "Missing required tool: $1"; }
need cmake
need git
need nproc || true
if ! command -v g++ >/dev/null 2>&1 && ! command -v clang++ >/dev/null 2>&1; then
  die "Need g++ or clang++"
fi
cmake --version | head -1

# --- Submodules ---
if [[ -f .gitmodules ]]; then
  yellow "Updating submodules (this may take a while)..."
  git submodule update --init --recursive \
    || yellow "Submodule update reported issues — continuing if sources exist"
fi

OUT="${OUT_DIR:-$ROOT/out}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
mkdir -p "$OUT"
cd "$OUT"

yellow "Configuring CMake ($BUILD_TYPE) → $OUT"
set +e
cmake "$ROOT" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ${CMAKE_EXTRA_ARGS:-}
CMAKE_RC=$?
set -e
if [[ $CMAKE_RC -ne 0 ]]; then
  red "CMake configure failed ($CMAKE_RC)."
  yellow "Install deps: https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-linux.md"
  yellow "Debian/Ubuntu example: build-essential cmake git ninja-build libgl-dev ... (see upstream docs)"
  exit "$CMAKE_RC"
fi

yellow "Building target Telegram with $JOBS jobs ($BUILD_TYPE)..."
set +e
cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram
BUILD_RC=$?
set -e
if [[ $BUILD_RC -ne 0 ]]; then
  red "Compile failed ($BUILD_RC). Scroll up for the first error."
  yellow "Retry with: BUILD_TYPE=Debug JOBS=2 $0"
  exit "$BUILD_RC"
fi

green "Build succeeded."
yellow "Looking for binaries..."
find "$OUT" -type f \( -name 'Drakeygram' -o -name 'Telegram' \) 2>/dev/null | head -20 || true
green "Done. Run the Drakeygram/Telegram binary from the out tree."
