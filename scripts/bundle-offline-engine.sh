#!/usr/bin/env bash
# Maintainer-only (build/package time). Ships a slim Bergamot CLI with the app.
# End users never run this. Keeps runtime free of engine downloads.
#
# Usage:
#   export DRAKEY_OFFLINE_ENGINE_BIN=/path/to/bergamot-translator
#   bash scripts/bundle-offline-engine.sh [install-prefix]
#
# Result:
#   <prefix>/offline-engine/bergamot-translator[.exe]
#
# Lightweight guidance:
#   - Prefer Release bergamot CLI built with -O2 / strip
#   - Do NOT statically link full Marian+MKL into Telegram (huge)
#   - Ship only the small CLI next to the app; models stay on-demand from Mozilla
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="${1:-$ROOT/out/install}"
NAME="bergamot-translator"
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*|Windows_NT) NAME="bergamot-translator.exe" ;;
esac

DEST="$PREFIX/offline-engine"
mkdir -p "$DEST"

if [[ -n "${DRAKEY_OFFLINE_ENGINE_BIN:-}" && -f "$DRAKEY_OFFLINE_ENGINE_BIN" ]]; then
  cp -f "$DRAKEY_OFFLINE_ENGINE_BIN" "$DEST/$NAME"
  chmod +x "$DEST/$NAME" 2>/dev/null || true
  echo "Bundled engine -> $DEST/$NAME"
  ls -lh "$DEST/$NAME"
  exit 0
fi

echo "Set DRAKEY_OFFLINE_ENGINE_BIN to a stripped Bergamot CLI build, then re-run."
echo "Build tip: cmake -DCMAKE_BUILD_TYPE=Release && strip the binary."
echo "App looks for: <applicationDir>/offline-engine/$NAME"
exit 1
