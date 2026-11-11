#!/usr/bin/env bash
# Drakeygram CLI adapter for Bergamot / translateLocally-style tools.
# Contract expected by OfflineMtEngine:
#   bergamot-wrapper.sh --model-dir DIR --from SRC --to TRG --input FILE
# Prints translation on stdout. Never uploads network data.
set -euo pipefail

MODEL_DIR=""
FROM=""
TO=""
INPUT=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --model-dir) MODEL_DIR="$2"; shift 2 ;;
    --from) FROM="$2"; shift 2 ;;
    --to) TO="$2"; shift 2 ;;
    --input) INPUT="$2"; shift 2 ;;
    *) shift ;;
  esac
done

[[ -n "$MODEL_DIR" && -n "$FROM" && -n "$TO" && -n "$INPUT" ]] || {
  echo "usage: $0 --model-dir DIR --from SRC --to TRG --input FILE" >&2
  exit 2
}
[[ -f "$INPUT" ]] || { echo "input missing: $INPUT" >&2; exit 1; }

# Prefer real bergamot binary if user set it and it's not this script
REAL="${DRAKEY_BERGAMOT_REAL:-}"
if [[ -z "$REAL" ]]; then
  for c in bergamot-translator bergamot translateLocally; do
    if command -v "$c" >/dev/null 2>&1; then
      REAL="$(command -v "$c")"
      break
    fi
  done
fi

if [[ -z "$REAL" || ! -x "$REAL" ]]; then
  echo "No Bergamot binary found. Set DRAKEY_BERGAMOT_REAL or install bergamot-translator." >&2
  exit 1
fi

# Try common CLI shapes (official builds differ).
if "$REAL" --help 2>&1 | grep -qiE 'model-dir|model_path|--model'; then
  exec "$REAL" --model-dir "$MODEL_DIR" --from "$FROM" --to "$TO" --input "$INPUT"
fi

# Fallback: pass text on stdin if binary reads stdin
if "$REAL" --help 2>&1 | grep -qi stdin; then
  exec "$REAL" --from "$FROM" --to "$TO" < "$INPUT"
fi

# Last resort: some builds take model path as positional
exec "$REAL" "$MODEL_DIR" "$FROM" "$TO" < "$INPUT"
