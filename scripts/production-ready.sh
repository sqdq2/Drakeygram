#!/usr/bin/env bash
# Production gate — fail closed, never disable features
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[WARN]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }

green "=== Drakeygram production-ready gate ==="

need_file() {
  local f="$1" min="${2:-1}"
  [[ -f "$f" ]] || die "Missing required file: $f"
  local sz; sz=$(wc -c <"$f" | tr -d ' ')
  [[ "$sz" -ge "$min" ]] || die "$f too small ($sz < $min) — possible truncate"
  green "$f ($sz bytes)"
}

need_file Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp 10000
need_file Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp 15000
need_file Telegram/CMakeLists.txt 50000
need_file Telegram/SourceFiles/ayu/security/session_guard.cpp 2000
need_file Telegram/SourceFiles/ayu/features/translator/offline/offline_mt_packs.cpp 2000
need_file Telegram/SourceFiles/ayu/features/translator/offline/offline_mt_engine.cpp 2000
need_file Telegram/SourceFiles/ayu/features/translator/implementations/offline.cpp 500
need_file Telegram/SourceFiles/ayu/features/translator/implementations/offline.h 400
need_file Telegram/SourceFiles/ayu/features/translator/ayu_translator.cpp 5000

for gone in \
  Telegram/SourceFiles/ayu/features/translator/implementations/google.cpp \
  Telegram/SourceFiles/ayu/features/translator/implementations/google.h \
  Telegram/SourceFiles/ayu/features/translator/implementations/yandex.cpp \
  Telegram/SourceFiles/ayu/features/translator/implementations/yandex.h
do
  if [[ -f "$gone" ]]; then
    die "Removed file still present: $gone — delete it"
  fi
done
green "google/yandex translator sources absent"

if grep -qE 'implementations/google\.(cpp|h)|implementations/yandex\.(cpp|h)' Telegram/CMakeLists.txt; then
  yellow "CMake still lists google/yandex — applying offline rename"
  bash scripts/apply-cmake-offline-rename.sh || die "apply-cmake-offline-rename failed"
fi
grep -q 'implementations/offline\.cpp' Telegram/CMakeLists.txt || {
  bash scripts/apply-cmake-offline-rename.sh || die "rename failed"
}
grep -q 'implementations/offline\.cpp' Telegram/CMakeLists.txt || die "CMake missing offline.cpp"
grep -qE 'implementations/google\.cpp|implementations/yandex\.cpp' Telegram/CMakeLists.txt && die "CMake still has google/yandex"
green "CMake translator sources OK"

if ! grep -q 'offline_mt_engine' Telegram/CMakeLists.txt; then
  bash scripts/apply-cmake-offline-mt.sh || true
fi
if ! grep -q 'session_guard' Telegram/CMakeLists.txt; then
  bash scripts/apply-cmake-session-guard.sh || true
fi
grep -q 'offline_mt_engine' Telegram/CMakeLists.txt || die "CMake missing offline_mt_engine"
grep -q 'session_guard' Telegram/CMakeLists.txt || die "CMake missing session_guard"
green "CMake offline_mt + session_guard OK"

if grep -RniE 'dpaste\.com|translate\.googleapis|translate-pa\.googleapis|translate\.yandex\.(com|net)|ayugram\.(one|org)' \
  Telegram/SourceFiles/ayu --include='*.cpp' --include='*.h' 2>/dev/null \
  | grep -vE 'disabled|removed|FORBIDDEN|no-op|clipboard only|bundled|GET only|permanently' ; then
  die "Forbidden live sink found in ayu sources"
fi
green "Sink scan clean"

bash scripts/security-audit.sh || die "security-audit failed"

green "=== GATE PASSED ==="
yellow "Build: ./scripts/build-linux.sh | ./scripts/build-macos.sh | ./scripts/build-windows.ps1"
