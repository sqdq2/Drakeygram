#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CMAKE="$ROOT/Telegram/CMakeLists.txt"
[[ -f "$CMAKE" ]] || exit 1
if grep -q 'session_guard.cpp' "$CMAKE"; then
  echo "session_guard already in CMakeLists"
  exit 0
fi
python3 - "$CMAKE" <<'PY'
import sys
path = sys.argv[1]
text = open(path, encoding='utf-8').read()
needle = "        ayu/features/translator/implementations/google.h\n"
add = needle + (
    "        ayu/security/session_guard.cpp\n"
    "        ayu/security/session_guard.h\n"
)
if "session_guard.cpp" in text:
    print("already present")
elif needle in text:
    open(path,"w",encoding="utf-8").write(text.replace(needle, add, 1))
    print("Inserted session_guard into CMakeLists")
else:
    # fallback: after any ayu/ line block
    print("Manual CMake insert required for session_guard.cpp/.h")
    raise SystemExit(1)
PY
