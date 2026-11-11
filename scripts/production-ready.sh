#!/usr/bin/env bash
# Production gate — must pass without disabling features
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }

green "=== Drakeygram production-ready (no feature compromise) ==="

check_min() {
  local f="$1" min="$2"
  [[ -f "$f" ]] || die "Missing $f"
  local sz; sz=$(wc -c <"$f")
  [[ "$sz" -ge "$min" ]] || die "$f too small ($sz < $min)"
  green "$f ($sz bytes)"
}

check_min Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp 10000
check_min Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp 15000
check_min Telegram/CMakeLists.txt 50000
check_min Telegram/SourceFiles/ayu/security/session_guard.cpp 2000
check_min Telegram/SourceFiles/ayu/features/translator/offline/offline_mt_packs.cpp 2000
check_min Telegram/SourceFiles/ayu/features/translator/offline/offline_mt_engine.cpp 2000

# Live sinks only (ignore docs/scripts mentioning forbidden names)
if grep -RniE 'dpaste\.com|translate\.googleapis|translate-pa\.googleapis|translate\.yandex|ayugram\.(one|org)' \
  Telegram/SourceFiles/ayu --include='*.cpp' --include='*.h' 2>/dev/null \
  | grep -vE 'disabled|production|clipboard only|no-op|FORBIDDEN|GET only|bundled' ; then
  die "Forbidden live sink in ayu sources"
fi
green "ayu sources sink scan clean"

if ! grep -q 'offline_mt_engine' Telegram/CMakeLists.txt; then
  yellow "CMake missing offline_mt_engine — applying extras"
  bash scripts/apply-all-cmake-extras.sh
fi
if ! grep -q 'session_guard' Telegram/CMakeLists.txt; then
  bash scripts/apply-all-cmake-extras.sh
fi
green "CMake lists offline engine + session_guard"

bash scripts/security-audit.sh

green "Gate OK — see OPTIMIZATION.md (safe opts only)"
yellow "Build: ./scripts/build-linux.sh | build-macos.sh | build-windows.ps1"
