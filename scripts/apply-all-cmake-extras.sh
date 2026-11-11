#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
bash scripts/apply-cmake-offline-mt.sh || true
bash scripts/apply-cmake-session-guard.sh || true
bash scripts/apply-cmake-offline-rename.sh || true
echo "Done applying CMake extras (offline MT, session_guard, offline rename)."
