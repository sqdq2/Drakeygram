#!/usr/bin/env bash
# Merge official Telegram Desktop (tdesktop) into Drakeygram.
# Preserves ayu/ fork features; re-applies CMake extras after merge.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m[ERROR]\033[0m %s\n' "$*" >&2; }
green(){ printf '\033[32m[OK]\033[0m %s\n' "$*"; }
yellow(){ printf '\033[33m[INFO]\033[0m %s\n' "$*"; }
die(){ red "$*"; exit 1; }

UPSTREAM_URL="${UPSTREAM_URL:-https://github.com/telegramdesktop/tdesktop.git}"
# Prefer official release tag when set, else upstream branch (default: dev)
UPSTREAM_REF="${UPSTREAM_REF:-}"
UPSTREAM_BRANCH="${UPSTREAM_BRANCH:-dev}"
MODE="${1:-merge}"  # merge | rebase | fetch-only

command -v git >/dev/null || die "git required"
[[ -d .git ]] || die "Run from a git clone of Drakeygram"

if ! git remote get-url upstream >/dev/null 2>&1; then
  yellow "Adding upstream remote: $UPSTREAM_URL"
  git remote add upstream "$UPSTREAM_URL"
else
  git remote set-url upstream "$UPSTREAM_URL"
fi

yellow "Fetching upstream (tags + branches)..."
git fetch upstream --tags --prune

# Resolve target
if [[ -n "$UPSTREAM_REF" ]]; then
  TARGET="$UPSTREAM_REF"
elif git rev-parse -q --verify "refs/tags/v7.4.4" >/dev/null 2>&1; then
  # Latest known stable at script update time; override with UPSTREAM_REF=vX.Y.Z
  TARGET="${FORCE_UPSTREAM_TAG:-v7.4.4}"
  if ! git rev-parse -q --verify "refs/tags/$TARGET" >/dev/null 2>&1; then
    TARGET="upstream/$UPSTREAM_BRANCH"
  fi
else
  TARGET="upstream/$UPSTREAM_BRANCH"
fi

# If user passed a tag as env
if [[ -n "${FORCE_UPSTREAM_TAG:-}" ]]; then
  TARGET="$FORCE_UPSTREAM_TAG"
fi

yellow "Upstream target: $TARGET"
git rev-parse "$TARGET" >/dev/null 2>&1 || die "Cannot resolve $TARGET — fetch failed?"

case "$MODE" in
  fetch-only)
    green "Fetch complete. Target tip: $(git rev-parse --short "$TARGET")"
    git log -1 --oneline "$TARGET"
    exit 0
    ;;
  rebase)
    yellow "Rebasing current branch onto $TARGET ..."
    git rebase "$TARGET" || {
      red "Rebase conflicts. Resolve, then: git rebase --continue"
      red "After done: bash scripts/apply-all-cmake-extras.sh && bash scripts/production-ready.sh"
      exit 1
    }
    ;;
  merge|*)
    yellow "Merging $TARGET into $(git branch --show-current) ..."
    if git merge-base --is-ancestor "$TARGET" HEAD 2>/dev/null; then
      green "Already up to date with $TARGET"
    else
      git merge "$TARGET" -m "merge: official Telegram Desktop ($TARGET)" || {
        red "Merge conflicts. Resolve files, then:"
        red "  git add -A && git commit"
        red "  bash scripts/apply-all-cmake-extras.sh"
        red "  bash scripts/production-ready.sh"
        exit 1
      }
    fi
    ;;
esac

yellow "Re-applying Drakeygram CMake extras (offline MT, session_guard, offline rename)..."
bash scripts/apply-all-cmake-extras.sh || die "apply-all-cmake-extras failed after upstream merge"

yellow "Running production gate..."
bash scripts/production-ready.sh || die "production-ready failed after upstream merge — fix before shipping"

green "Upstream sync complete."
green "Next: commit any CMake/script changes if needed, then ./scripts/build-linux.sh"
