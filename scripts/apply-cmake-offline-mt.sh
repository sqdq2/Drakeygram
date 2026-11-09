#!/usr/bin/env bash
# Inserts offline_mt_packs sources into Telegram/CMakeLists.txt if missing.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CMAKE="$ROOT/Telegram/CMakeLists.txt"
[[ -f "$CMAKE" ]] || { echo "missing $CMAKE"; exit 1; }
if grep -q 'offline_mt_packs.cpp' "$CMAKE"; then
  echo "offline_mt_packs already in CMakeLists.txt"
  exit 0
fi
if grep -q 'ayu/features/translator/implementations/google.h' "$CMAKE"; then
  # portable in-place insert after google.h line
  python3 - "$CMAKE" <<'PY'
import sys
path = sys.argv[1]
text = open(path, encoding='utf-8').read()
needle = "        ayu/features/translator/implementations/google.h\n"
add = needle + "        ayu/features/translator/offline/offline_mt_packs.cpp\n        ayu/features/translator/offline/offline_mt_packs.h\n"
if needle not in text:
    raise SystemExit('google.h line not found')
open(path,'w',encoding='utf-8').write(text.replace(needle, add, 1))
print('Inserted offline_mt_packs into', path)
PY
else
  echo "Could not find google.h entry in CMakeLists.txt"
  exit 1
fi
