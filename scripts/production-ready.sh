#!/usr/bin/env bash
# One-shot production gate: cmake insert, security audit, optional build.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }

green "=== Drakeygram production-ready gate ==="

# 1) Critical file size checks
check_min() {
  local f="$1" min="$2"
  [[ -f "$f" ]] || die "Missing $f"
  local sz
  sz=$(wc -c <"$f")
  if [[ "$sz" -lt "$min" ]]; then
    die "$f is too small ($sz bytes) — likely truncated; restore production content"
  fi
  green "$f size OK ($sz)"
}
check_min Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp 10000
check_min Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp 15000
check_min Telegram/CMakeLists.txt 50000

# 2) Forbidden sinks
if grep -RniE 'dpaste\.com|translate\.googleapis|translate\.yandex|ayugram\.one' \
  Telegram/SourceFiles/ayu --include='*.cpp' --include='*.h' 2>/dev/null \
  | grep -vE 'disabled|production|GET only|clipboard only|no-op'; then
  die "Forbidden network sink detected in ayu/ sources"
fi
green "No forbidden sinks in quick scan"

# 3) CMake offline MT
bash scripts/apply-cmake-offline-mt.sh

# 4) Security audit
bash scripts/security-audit.sh

# 5) Optional full build
if [[ "${RUN_BUILD:-0}" == "1" ]]; then
  yellow "RUN_BUILD=1 → starting platform build"
  if [[ "$(uname -s)" == "Darwin" ]]; then
    bash scripts/build-macos.sh
  else
    bash scripts/build-linux.sh
  fi
else
  yellow "Skip compile (set RUN_BUILD=1 to build now)"
  yellow "Next: ./scripts/build-linux.sh  OR  build-macos.sh  OR  build-windows.ps1"
fi

green "Production gate finished"
