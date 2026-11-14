#!/usr/bin/env bash
# Update Drakeygram core from official tdesktop WITHOUT requiring shared git history.
# Your repo is independent (not a GitHub fork), so `git merge upstream` conflicts massively.
# This script overlays upstream sources and PRESERVES Drakeygram ayu/ + scripts/docs.
#
# Usage:
#   bash scripts/update-from-tdesktop.sh v7.4.4
#   bash scripts/update-from-tdesktop.sh           # defaults to v7.4.4
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }

TAG="${1:-v7.4.4}"
UPSTREAM_URL="${UPSTREAM_URL:-https://github.com/telegramdesktop/tdesktop.git}"
WORK="${DRAKEY_UPSTREAM_WORK:-$ROOT/.upstream-tdesktop}"

command -v git >/dev/null || die "git required"
command -v rsync >/dev/null || die "rsync required (install rsync)"

[[ -d .git ]] || die "Run inside a Drakeygram git clone"

yellow "Updating core from official tdesktop $TAG ..."
yellow "Preserving: Telegram/SourceFiles/ayu, scripts/, docs/, README, SECURITY, OPTIMIZATION"

if [[ ! -d "$WORK/.git" ]]; then
  yellow "Cloning upstream (depth 1, tag $TAG)..."
  rm -rf "$WORK"
  git clone --depth 1 --branch "$TAG" "$UPSTREAM_URL" "$WORK" \
    || git clone --depth 1 "$UPSTREAM_URL" "$WORK" && (cd "$WORK" && git fetch --depth 1 origin tag "$TAG" && git checkout "$TAG")
else
  yellow "Reusing $WORK"
  (cd "$WORK" && git fetch --tags --depth 1 origin tag "$TAG" && git checkout "$TAG")
fi

(cd "$WORK" && git submodule update --init --recursive) || yellow "Submodule init had warnings — continue"

# Backup ayu tree
BAK="$ROOT/.drakey-preserve-$$"
mkdir -p "$BAK"
[[ -d Telegram/SourceFiles/ayu ]] && cp -a Telegram/SourceFiles/ayu "$BAK/ayu"
[[ -d scripts ]] && cp -a scripts "$BAK/scripts"

yellow "Overlay upstream Telegram/ cmake/ into repo (excluding ayu)..."
# Core trees from upstream
for dir in cmake Telegram; do
  if [[ -d "$WORK/$dir" ]]; then
    mkdir -p "$ROOT/$dir"
    rsync -a --delete \
      --exclude 'SourceFiles/ayu/' \
      --exclude 'SourceFiles/ayu' \
      "$WORK/$dir/" "$ROOT/$dir/"
  fi
done

# Top-level build files commonly needed
for f in CMakeLists.txt changelog.txt LEGAL LICENSE README.md \
         .gitmodules snap/snapcraft.yaml; do
  if [[ -e "$WORK/$f" ]]; then
    mkdir -p "$(dirname "$ROOT/$f")"
    rsync -a "$WORK/$f" "$ROOT/$f"
  fi
done

# Restore Drakeygram-specific trees
if [[ -d "$BAK/ayu" ]]; then
  mkdir -p Telegram/SourceFiles
  rm -rf Telegram/SourceFiles/ayu
  cp -a "$BAK/ayu" Telegram/SourceFiles/ayu
  green "Restored Telegram/SourceFiles/ayu"
else
  yellow "No ayu backup found — unexpected"
fi
if [[ -d "$BAK/scripts" ]]; then
  # Keep upstream scripts if any, but force our scripts on top
  rsync -a "$BAK/scripts/" "$ROOT/scripts/"
  green "Restored scripts/"
fi
rm -rf "$BAK"

yellow "Re-applying Drakeygram CMake extras..."
bash scripts/apply-all-cmake-extras.sh || die "apply-all-cmake-extras failed"

yellow "Production gate..."
bash scripts/production-ready.sh || die "production-ready failed after overlay"

green "Overlay from $TAG complete."
yellow "Review: git status"
yellow "Commit when satisfied, then build:"
yellow "  git add -A && git commit -m 'update: overlay tdesktop $TAG'"
yellow "  ./scripts/build-linux.sh"
