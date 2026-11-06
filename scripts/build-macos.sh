#!/usr/bin/env bash
# Drakeygram — macOS all-in-one build
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

red() { printf '\033[31m%s\033[0m\n' "$*"; }
green() { printf '\033[32m%s\033[0m\n' "$*"; }
yellow() { printf '\033[33m%s\033[0m\n' "$*"; }
die() { red "ERROR: $*"; exit 1; }

green "=== Drakeygram macOS build ==="

if [[ -z "${TELEGRAM_API_ID:-}" || -z "${TELEGRAM_API_HASH:-}" ]]; then
  read -r -p "Telegram API ID: " TELEGRAM_API_ID
  read -r -p "Telegram API Hash: " TELEGRAM_API_HASH
fi
[[ "$TELEGRAM_API_ID" =~ ^[0-9]+$ ]] || die "API ID must be numeric (https://my.telegram.org)"
[[ -n "$TELEGRAM_API_HASH" ]] || die "API Hash is required"

need_cmd() { command -v "$1" >/dev/null 2>&1 || die "Missing: $1"; }
need_cmd cmake
need_cmd git
need_cmd xcodebuild

if [[ -f .gitmodules ]]; then
  git submodule update --init --recursive || true
fi

OUT="${OUT_DIR:-$ROOT/out}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

mkdir -p "$OUT"
cd "$OUT"

yellow "Configure..."
cmake "$ROOT" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  ${CMAKE_EXTRA_ARGS:-} \
  || die "CMake failed. See https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-mac.md"

yellow "Build -j$JOBS..."
cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram \
  || die "Build failed"

green "Done. Look for Drakeygram.app / Telegram under $OUT"
find "$OUT" -name 'Drakeygram.app' -o -name 'Telegram.app' 2>/dev/null | head -10 || true
