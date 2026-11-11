#!/usr/bin/env bash
# Drakeygram Linux Release build — optimized, feature-complete
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

Jobs="${DRAKEY_BUILD_JOBS:-$(nproc 2>/dev/null || echo 4)}"
BuildType="${CMAKE_BUILD_TYPE:-Release}"

echo "=== Drakeygram Linux build ($BuildType, -j$Jobs) ==="

if [[ -z "${TELEGRAM_API_ID:-}" ]]; then
  read -r -p "Telegram API ID: " TELEGRAM_API_ID
fi
if [[ -z "${TELEGRAM_API_HASH:-}" ]]; then
  read -r -p "Telegram API Hash: " TELEGRAM_API_HASH
fi

if [[ -z "${TELEGRAM_API_ID:-}" || -z "${TELEGRAM_API_HASH:-}" ]]; then
  echo "API ID and Hash are required (https://my.telegram.org)" >&2
  exit 1
fi

bash scripts/apply-all-cmake-extras.sh || true
bash scripts/security-audit.sh || {
  echo "security-audit failed — fix sinks before shipping" >&2
  exit 1
}

mkdir -p build/linux
cd build/linux

CMAKE_EXTRA=()
if [[ "${DRAKEY_ENABLE_LTO:-0}" == "1" ]]; then
  CMAKE_EXTRA+=("-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON")
  echo "LTO enabled (DRAKEY_ENABLE_LTO=1)"
fi

cmake ../.. \
  -DCMAKE_BUILD_TYPE="$BuildType" \
  -DTDESKTOP_API_ID="$TELEGRAM_API_ID" \
  -DTDESKTOP_API_HASH="$TELEGRAM_API_HASH" \
  "${CMAKE_EXTRA[@]}"

cmake --build . -j"$Jobs"

echo "=== Build finished ==="
echo "Optional: DRAKEY_OFFLINE_ENGINE_BIN=... bash scripts/bundle-offline-engine.sh <prefix>"
