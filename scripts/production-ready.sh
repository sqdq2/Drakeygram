#!/usr/bin/env bash
# Production gate v2 — sizes, sinks, cmake extras, security audit
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }

green "=== Drakeygram production-ready gate v2 ==="

check_min() {
  local f="$1" min="$2"
  [[ -f "$f" ]] || die "Missing $f"
  local sz; sz=$(wc -c <"$f")
  if [[ "$sz" -lt "$min" ]]; then
    die "$f too small ($sz < $min) — restore full production file"
  fi
  green "$f ($sz bytes)"
}

check_min Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp 10000
check_min Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp 15000
check_min Telegram/CMakeLists.txt 50000
check_min Telegram/SourceFiles/ayu/security/session_guard.cpp 2000
check_min Telegram/SourceFiles/ayu/features/translator/offline/offline_mt_packs.cpp 2000

if grep -RniE 'dpaste\.com|translate\.googleapis|translate\.yandex|ayugram\.one' \
  Telegram/SourceFiles/ayu --include='*.cpp' --include='*.h' 2>/dev/null \
  | grep -vE 'disabled|production|GET only|clipboard only|no-op|FORBIDDEN' ; then
  die "Forbidden network sink detected"
fi
green "Quick sink scan clean"

bash scripts/apply-all-cmake-extras.sh
bash scripts/security-audit.sh

if [[ "${RUN_BUILD:-0}" == "1" ]]; then
  if [[ "$(uname -s)" == "Darwin" ]]; then bash scripts/build-macos.sh
  else bash scripts/build-linux.sh
  fi
else
  yellow "Set RUN_BUILD=1 to compile now"
fi

green "Gate finished — see docs/FEATURE_MATRIX.md"
