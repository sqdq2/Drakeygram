#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CMAKE="$ROOT/Telegram/CMakeLists.txt"
[[ -f "$CMAKE" ]] || { echo "missing CMakeLists"; exit 1; }

python3 - "$CMAKE" <<'PY'
from pathlib import Path
import sys
path = Path(sys.argv[1])
text = path.read_text(encoding="utf-8")
extras = [
    "        ayu/features/translator/offline/offline_mt_packs.cpp\n",
    "        ayu/features/translator/offline/offline_mt_packs.h\n",
    "        ayu/features/translator/offline/offline_mt_engine.cpp\n",
    "        ayu/features/translator/offline/offline_mt_engine.h\n",
    "        ayu/security/session_guard.cpp\n",
    "        ayu/security/session_guard.h\n",
]
anchor = "        ayu/features/translator/implementations/google.h\n"
if anchor not in text:
    print("anchor google.h not found — add extras manually")
    sys.exit(1)
block = "".join(extras)
# remove existing extras to avoid duplicates
for e in extras:
    text = text.replace(e, "")
text = text.replace(anchor, anchor + block, 1)
path.write_text(text, encoding="utf-8")
print("CMake extras applied:", len(extras), "entries")
PY
