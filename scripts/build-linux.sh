#!/usr/bin/env bash
# Drakeygram — production Linux build
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }
trap 'die "Failed at line $LINENO"' ERR

green "=== Drakeygram production build (Linux) ==="

# Ensure offline MT sources listed in CMake
if [[ -x "$ROOT/scripts/apply-cmake-offline-mt.sh" ]] || [[ -f "$ROOT/scripts/apply-cmake-offline-mt.sh" ]]; then
  bash "$ROOT/scripts/apply-cmake-offline-mt.sh" || yellow "CMake offline_mt insert skipped"
fi

if [[ -z "${TELEGRAM_API_ID:-}" ]]; then read -r -p "Telegram API ID: " TELEGRAM_API_ID || true; fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then read -r -p "Telegram API Hash: " TELEGRAM_API_HASH || true; fi
[[ "${TELEGRAM_API_ID:-}" =~ ^[0-9]+$ ]] || die "API ID must be numeric — https://my.telegram.org"
[[ -n "${TELEGRAM_API_HASH:-}" ]] || die "API Hash required — https://my.telegram.org"
export TELEGRAM_API_ID TELEGRAM_API_HASH

command -v cmake >/dev/null || die "cmake not found"
command -v git >/dev/null || die "git not found"
if ! command -v g++ >/dev/null && ! command -v clang++ >/dev/null; then die "g++ or clang++ required"; fi

if [[ -f .gitmodules ]]; then
  yellow "Submodules..."
  git submodule update --init --recursive || yellow "submodule warnings"
fi

OUT="${OUT_DIR:-$ROOT/out}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
mkdir -p "$OUT" && cd "$OUT"

yellow "Configure $BUILD_TYPE..."
cmake "$ROOT" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ${CMAKE_EXTRA_ARGS:-} \
  || die "CMake failed — install deps from tdesktop Linux build docs"

yellow "Build -j$JOBS..."
cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram \
  || die "Compile failed"

green "Build OK"
find "$OUT" -type f \( -name Drakeygram -o -name Telegram \) 2>/dev/null | head -20 || true
