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
if [[ -z "${TELEGRAM_API_ID:-}" ]]; then read -r -p "Telegram API ID: " TELEGRAM_API_ID || true; fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then read -r -p "Telegram API Hash: " TELEGRAM_API_HASH || true; fi
[[ "${TELEGRAM_API_ID:-}" =~ ^[0-9]+$ ]] || die "API ID must be numeric — https://my.telegram.org"
[[ -n "${TELEGRAM_API_HASH:-}" ]] || die "API Hash required — https://my.telegram.org"
export TELEGRAM_API_ID TELEGRAM_API_HASH
need() { command -v "$1" >/dev/null 2>&1 || die "Missing required tool: $1"; }
need cmake; need git
if ! command -v g++ >/dev/null 2>&1 && ! command -v clang++ >/dev/null 2>&1; then die "Need g++ or clang++"; fi
if [[ -f .gitmodules ]]; then git submodule update --init --recursive || yellow "Submodule warnings"; fi
OUT="${OUT_DIR:-$ROOT/out}"; BUILD_TYPE="${BUILD_TYPE:-Release}"; JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
mkdir -p "$OUT"; cd "$OUT"
yellow "Configuring CMake ($BUILD_TYPE)..."
set +e
cmake "$ROOT" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DTDESKTOP_API_ID="$TELEGRAM_API_ID" -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${CMAKE_EXTRA_ARGS:-}
CMAKE_RC=$?; set -e
[[ $CMAKE_RC -eq 0 ]] || die "CMake failed — see tdesktop Linux build docs"
yellow "Building ($JOBS jobs)..."
set +e; cmake --build . --config "$BUILD_TYPE" -j"$JOBS" --target Telegram; BUILD_RC=$?; set -e
[[ $BUILD_RC -eq 0 ]] || die "Compile failed"
green "Build succeeded."
find "$OUT" -type f \( -name 'Drakeygram' -o -name 'Telegram' \) 2>/dev/null | head -20 || true
