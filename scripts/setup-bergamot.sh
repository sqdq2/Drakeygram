#!/usr/bin/env bash
# Optional: prepare local Bergamot offline decode for Drakeygram.
# Does NOT download huge model packs (app downloads those on demand).
# Installs/locates a local decode binary only.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
red(){ printf '\033[31m[ERR]\033[0m %s\n' "$*" >&2; }

yellow "Drakeygram optional offline decode setup"
yellow "Privacy: decode is local only; packs are HTTPS GET when you download them in-app."

DEST="${DRAKEY_BERGAMOT_DIR:-$ROOT/DrakeygramBergamot}"
mkdir -p "$DEST"

WRAPPER="$ROOT/scripts/bergamot-wrapper.sh"
chmod +x "$WRAPPER" 2>/dev/null || true

# Copy wrapper into local helper dir for auto-discovery by the app
cp -f "$WRAPPER" "$DEST/bergamot-translator" 2>/dev/null || true
chmod +x "$DEST/bergamot-translator" 2>/dev/null || true

if command -v bergamot-translator >/dev/null 2>&1; then
  green "Found bergamot-translator: $(command -v bergamot-translator)"
  echo "export DRAKEY_BERGAMOT_BIN=$(command -v bergamot-translator)"
elif command -v bergamot >/dev/null 2>&1; then
  green "Found bergamot: $(command -v bergamot)"
  echo "export DRAKEY_BERGAMOT_BIN=$(command -v bergamot)"
else
  yellow "No Bergamot binary on PATH yet."
  yellow "Build or install from: https://github.com/browsermt/bergamot-translator"
  yellow "Or use Firefox translateLocally / Browser MT builds."
  yellow "Then either:"
  echo "  export DRAKEY_BERGAMOT_BIN=/path/to/bergamot-translator"
  echo "  # or place binary at: $DEST/bergamot-translator"
fi

# App also looks under profile working dir DrakeygramBergamot/
yellow "Wrapper installed at: $DEST/bergamot-translator"
yellow "Point DRAKEY_BERGAMOT_REAL at the real engine if using the wrapper."
echo
green "Setup notes written. Offline packs still download from Mozilla inside the app (user action only)."
