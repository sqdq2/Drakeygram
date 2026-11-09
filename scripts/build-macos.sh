#!/usr/bin/env bash
# Drakeygram — production macOS build
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"; cd "$ROOT"
red()    { printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green()  { printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow() { printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die()    { red "$*"; exit 1; }
trap 'die "Build failed at line $LINENO"' ERR
green "=== Drakeygram macOS production build ==="
if [[ -z "${TELEGRAM_API_ID:-}" ]]; then read -r -p "Telegram API ID: " TELEGRAM_API_ID || true; fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then read -r -p "Telegram API Hash: " TELEGRAM_API_HASH || true; fi
[[ "${TELEGRAM_API_ID:-}" =~ ^[0-9]+$ ]] || die "API ID must be numeric"
[[ -n "${TELEGRAM_API_HASH:-}" ]] || die "API Hash required"
export TELEGRAM_API_ID TELEGRAM_API_HASH
command -v cmake >/dev/null && command -v git >/dev/null && command -v xcodebuild >/dev/null || die "Need cmake, git, xcodebuild"
[[ -f .gitmodules ]] && git submodule update --init --recursive || true
OUT="${OUT_DIR:-$ROOT/out}"; BUILD_TYPE="${BUILD_TYPE:-Release}"; JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
mkdir -p "$OUT"; cd "$OUT"
cmake "$ROOT" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DTDESKTOP_API_ID="$TELEGRAM_API_ID" -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" ${CMAKE_EXTRA_ARGS:-} || die "CMake failed — see tdesktop macOS docs"
cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram || die "Build failed"
green "Done."; find "$OUT" \( -name 'Drakeygram.app' -o -name 'Telegram.app' \) 2>/dev/null | head -10 || true
