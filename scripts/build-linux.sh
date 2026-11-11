#!/usr/bin/env bash
# Drakeygram Linux Release — hardened error handling
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

Jobs="${DRAKEY_BUILD_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
BuildType="${CMAKE_BUILD_TYPE:-Release}"

echo "=== Drakeygram Linux build ($BuildType, -j$Jobs) ==="

if ! command -v cmake >/dev/null 2>&1; then
  echo "[ERROR] cmake not found in PATH" >&2
  exit 1
fi

if [[ -z "${TELEGRAM_API_ID:-}" ]]; then
  read -r -p "Telegram API ID: " TELEGRAM_API_ID || true
fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then
  read -r -p "Telegram API Hash: " TELEGRAM_API_HASH || true
fi
if [[ -z "${TELEGRAM_API_ID:-}" || -z "${TELEGRAM_API_HASH:-}" ]]; then
  echo "[ERROR] API ID and Hash required (https://my.telegram.org)" >&2
  exit 1
fi
if ! [[ "$TELEGRAM_API_ID" =~ ^[0-9]+$ ]]; then
  echo "[ERROR] API ID must be numeric" >&2
  exit 1
fi
if [[ ${#TELEGRAM_API_HASH} -lt 16 ]]; then
  echo "[ERROR] API Hash looks too short" >&2
  exit 1
fi

bash scripts/apply-all-cmake-extras.sh || {
  echo "[ERROR] apply-all-cmake-extras failed" >&2
  exit 1
}
bash scripts/production-ready.sh || {
  echo "[ERROR] production-ready gate failed — fix before build" >&2
  exit 1
}

mkdir -p build/linux
cd build/linux

CMAKE_EXTRA=()
if [[ "${DRAKEY_ENABLE_LTO:-0}" == "1" ]]; then
  CMAKE_EXTRA+=("-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON")
  echo "LTO enabled (DRAKEY_ENABLE_LTO=1)"
fi

set +e
cmake ../.. \
  -DCMAKE_BUILD_TYPE="$BuildType" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  "${CMAKE_EXTRA[@]}"
cmake_rc=$?
set -e
if [[ $cmake_rc -ne 0 ]]; then
  echo "[ERROR] cmake configure failed (exit $cmake_rc)" >&2
  echo "Hint: install Qt/deps; git submodule update --init --recursive" >&2
  exit "$cmake_rc"
fi

set +e
cmake --build . -j"$Jobs"
build_rc=$?
set -e
if [[ $build_rc -ne 0 ]]; then
  echo "[ERROR] compile failed (exit $build_rc)" >&2
  echo "Hint: run scripts/apply-cmake-offline-rename.sh if offline sources missing" >&2
  exit "$build_rc"
fi

echo "=== Build finished OK ==="
echo "Optional: DRAKEY_OFFLINE_ENGINE_BIN=... bash scripts/bundle-offline-engine.sh <prefix>"
