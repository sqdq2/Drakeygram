#!/usr/bin/env bash
# Drakeygram — production macOS build (all-in-one)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

red()    { printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green()  { printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow() { printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die()    { red "$*"; exit 1; }
trap 'die "Build failed at line $LINENO (exit $?)"' ERR

green "=== Drakeygram macOS production build ==="

if [[ -z "${TELEGRAM_API_ID:-}" ]]; then
  read -r -p "Telegram API ID: " TELEGRAM_API_ID || true
fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then
  read -r -p "Telegram API Hash: " TELEGRAM_API_HASH || true
fi
[[ "${TELEGRAM_API_ID:-}" =~ ^[0-9]+$ ]] || die "API ID must be numeric — https://my.telegram.org"
[[ -n "${TELEGRAM_API_HASH:-}" ]] || die "API Hash required — https://my.telegram.org"
export TELEGRAM_API_ID TELEGRAM_API_HASH

need() { command -v "$1" >/dev/null 2>&1 || die "Missing: $1"; }
need cmake
need git
need xcodebuild
xcodebuild -version | head -2 || true

if [[ -f .gitmodules ]]; then
  yellow "Updating submodules..."
  git submodule update --init --recursive || yellow "Submodule warnings ignored"
fi

OUT="${OUT_DIR:-$ROOT/out}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
mkdir -p "$OUT"
cd "$OUT"

yellow "Configure ($BUILD_TYPE)..."
set +e
cmake "$ROOT" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  ${CMAKE_EXTRA_ARGS:-}
CMAKE_RC=$?
set -e
if [[ $CMAKE_RC -ne 0 ]]; then
  red "CMake failed. See: https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-mac.md"
  exit "$CMAKE_RC"
fi

yellow "Build -j$JOBS..."
set +e
cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram
BUILD_RC=$?
set -e
if [[ $BUILD_RC -ne 0 ]]; then
  red "Build failed ($BUILD_RC)"
  exit "$BUILD_RC"
fi

green "Build succeeded."
find "$OUT" \( -name 'Drakeygram.app' -o -name 'Telegram.app' \) 2>/dev/null | head -10 || true
green "Done."
