#!/usr/bin/env bash
# Future-proof: ensure all Drakeygram extra sources are listed in CMakeLists.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
bash scripts/apply-cmake-offline-mt.sh || true
bash scripts/apply-cmake-session-guard.sh || true
echo "Done applying CMake extras."
