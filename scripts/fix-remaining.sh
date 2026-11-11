#!/usr/bin/env bash
# Run from repo root after git pull. Fixes remaining production blockers on disk.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m%s\033[0m\n' "$*" >&2; }
green(){ printf '\033[32m%s\033[0m\n' "$*"; }

# 1) filters_utils: kill dpaste + arbitrary URL import
FU="Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp"
if [[ -f "$FU" ]] && grep -q 'dpaste.com' "$FU"; then
  python3 - "$FU" <<'PY'
import re, sys
path = sys.argv[1]
text = open(path, encoding='utf-8').read()
text, n1 = re.subn(
    r'void FilterUtils::importFromLink\(const QString &link\) \{.*?\n\}',
    '''void FilterUtils::importFromLink(const QString &link) {\n\tQ_UNUSED(link);\n\tUi::Toast::Show(tr::ayu_FiltersToastFailFetch(tr::now));\n\tLOG(("FilterUtils: importFromLink disabled (production)"));\n}''',
    text, count=1, flags=re.S)
text, n2 = re.subn(
    r'void FilterUtils::publishFilters\(\) \{.*?\n\}',
    '''void FilterUtils::publishFilters() {\n\tconst auto exported = exportFilters();\n\tif (exported.isEmpty()) {\n\t\tUi::Toast::Show(tr::ayu_FiltersToastFailPublish(tr::now));\n\t\treturn;\n\t}\n\tQGuiApplication::clipboard()->setText(exported);\n\tUi::Toast::Show(tr::lng_stickers_copied(tr::now));\n\tLOG(("FilterUtils: publishFilters clipboard only (production)"));\n}''',
    text, count=1, flags=re.S)
open(path, 'w', encoding='utf-8').write(text)
print(f'filters patched import={n1} publish={n2} dpaste_left={"dpaste.com" in text}')
if n1 != 1 or n2 != 1:
    raise SystemExit('filters patch failed')
PY
  green "filters_utils network sinks removed"
else
  green "filters_utils already clean or missing"
fi

# 2) plugin_info_box: restore from known-good commit if stub
PI="Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp"
if [[ -f "$PI" ]] && [[ $(wc -c <"$PI") -lt 5000 ]]; then
  curl -fsSL "https://raw.githubusercontent.com/sqdq2/Drakeygram/ec2c67f6f9eeb0fa0e3f0683fc9abe6b07693fdd/Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp" -o "$PI"
  python3 - "$PI" <<'PY'
from pathlib import Path
import sys
p = Path(sys.argv[1])
t = p.read_text(encoding='utf-8')
old = '''\t\t\tchip->setClickedCallback([dep] {\n\t\t\t\tFile::OpenUrl(\n\t\t\t\t\tu"https://pypi.org/project/"_q + dep + u"/"_q);\n\t\t\t});'''
new = '''\t\t\tchip->setClickedCallback([dep] {\n\t\t\t\tQ_UNUSED(dep);\n\t\t\t});'''
if old in t:
    t = t.replace(old, new)
    p.write_text(t, encoding='utf-8')
    print('plugin pypi removed')
else:
    print('plugin restored; pypi pattern check:', 'pypi.org' in t)
PY
  green "plugin_info_box restored"
else
  green "plugin_info_box size OK or missing"
fi

# 3) CMake extras
bash scripts/apply-all-cmake-extras.sh

green "fix-remaining done. Commit changes if you want them on GitHub:"
echo "  git add -A && git commit -m 'fix: production blockers' && git push"
