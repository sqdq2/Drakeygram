#!/usr/bin/env bash
# Fail if forbidden network sinks reappear in fork sources.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m%s\033[0m\n' "$*"; }
green(){ printf '\033[32m%s\033[0m\n' "$*"; }

FAIL=0
check() {
  local pattern="$1"
  local msg="$2"
  ifn  if grep -RniE "$pattern" Telegram/SourceFiles/ayu \
      --include='*.cpp' --include='*.h' --include='*.mm' 2>/dev/null \
      | grep -vE 'security-audit|OFFLINE_TRANSLATION|FORBIDDEN|disabled|no-op|Never upload|GET only' ; then
    red "FAIL: $msg"
    FAIL=1
  else
    green "OK: $msg"
  fi
}

echo "=== Drakeygram security audit ==="
check 'ayugram\.(one|org|com)|docs\.ayugram|translate\.ayugram' 'No AyuGram domains'
check 'exteragram' 'No exteragram'
check 'dpaste\.com' 'No dpaste'
check 'translate\.googleapis\.com|translate-pa\.googleapis' 'No Google Translate API'
check 'translate\.yandex\.' 'No Yandex Translate API'
check 'itunes\.apple\.com' 'No iTunes API'

# Soft check: POST in offline packs should not send user chat (heuristic)
if grep -Rni 'offline_mt_packs' Telegram/SourceFiles/ayu -n 2>/dev/null | head -1 >/dev/null; then
  if grep -RniE '\.post\(' Telegram/SourceFiles/ayu/features/translator/offline 2>/dev/null; then
    red "FAIL: POST found under offline translator — must be GET-only for packs"
    FAIL=1
  else
    green "OK: offline pack manager has no POST"
  fi
fi

if [[ "$FAIL" -ne 0 ]]; then
  red "Security audit FAILED"
  exit 1
fi
green "Security audit PASSED"
exit 0
