#!/usr/bin/env bash
# Replace google/yandex translator sources with offline in CMakeLists.txt
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CMAKE="$ROOT/Telegram/CMakeLists.txt"
python3 - "$CMAKE" <<'PY'
from pathlib import Path
import sys
p = Path(sys.argv[1])
t = p.read_text(encoding='utf-8')
for s in [
    '        ayu/features/translator/implementations/google.cpp\n',
    '        ayu/features/translator/implementations/google.h\n',
    '        ayu/features/translator/implementations/yandex.cpp\n',
    '        ayu/features/translator/implementations/yandex.h\n',
]:
    t = t.replace(s, '')
off = (
    '        ayu/features/translator/implementations/offline.cpp\n'
    '        ayu/features/translator/implementations/offline.h\n'
)
if 'implementations/offline.cpp' not in t:
    anchor = '        ayu/features/translator/offline/offline_mt_packs.cpp\n'
    if anchor in t:
        t = t.replace(anchor, off + anchor, 1)
    else:
        anchor2 = '        ayu/features/translator/ayu_translator.cpp\n'
        if anchor2 in t:
            t = t.replace(anchor2, anchor2 + off, 1)
        else:
            raise SystemExit('no insert anchor')
p.write_text(t, encoding='utf-8')
print('CMake offline rename applied')
PY
