#!/usr/bin/env bash
# Drakeygram — Linux all-in-one build
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

red() { printf '\033[31m%s\033[0m\n' "$*"; }
green() { printf '\033[32m%s\033[0m\n' "$*"; }
yellow() { printf '\033[33m%s\033[0m\n' "$*"; }

die() { red "ERROR: $*"; exit 1; }

green "=== Drakeygram Linux build ==="

# --- API credentials ---
if [[ -z "${TELEGRAM_API_ID:-}" || -z "${TELEGRAM_API_HASH:-}" ]]; then
  read -r -p "Telegram API ID: " TELEGRAM_API_ID
  read -r -p "Telegram API Hash: " TELEGRAM_API_HASH
fi
[[ "$TELEGRAM_API_ID" =~ ^[0-9]+$ ]] || die "API ID must be numeric (get from https://my.telegram.org)"
[[ -n "$TELEGRAM_API_HASH" ]] || die "API Hash is required"

export TELEGRAM_API_ID TELEGRAM_API_HASH

# --- Dependencies check ---
need_cmd() { command -v "$1" >/dev/null 2>&1 || die "Missing command: $1"; }
need_cmd cmake
need_cmd git
need_cmd g++ || need_cmd clang++

yellow "Checking build tools..."
cmake --version | head -1

# Common packages hint
if ! pkg-config --exists Qt6Core 2>/dev/null && ! pkg-config --exists Qt5Core 2>/dev/null; then
  yellow "Qt not found via pkg-config. Install Qt6 + deps per tdesktop docs:"
  yellow "  https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-linux.md"
fi

# --- Submodules ---
if [[ -f .gitmodules ]]; then
  yellow "Updating submodules..."
  git submodule update --init --recursive || yellow "Submodule update had warnings (continuing)"
fi

OUT="${OUT_DIR:-$ROOT/out}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

mkdir -p "$OUT"
cd "$OUT"

yellow "Configuring CMake ($BUILD_TYPE)..."
cmake "$ROOT" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  ${CMAKE_EXTRA_ARGS:-} \
  || die "CMake configure failed. Install dependencies from tdesktop Linux build docs."

yellow "Building (jobs=$JOBS)..."
cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram \
  || die "Build failed. Check errors above."

green "Build finished."
green "Binary is under: $OUT (search for Drakeygram or Telegram executable)"
find "$OUT" -type f -name 'Drakeygram' -o -name 'Telegram' 2>/dev/null | head -20 || true
