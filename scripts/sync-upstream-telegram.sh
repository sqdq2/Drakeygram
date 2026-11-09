#!/usr/bin/env bash
# Merge official Telegram Desktop (tdesktop) into this fork for security/feature updates.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

red(){ printf '\033[31m%s\033[0m\n' "$*" >&2; }
green(){ printf '\033[32m%s\033[0m\n' "$*"; }
yellow(){ printf '\033[33m%s\033[0m\n' "$*"; }
die(){ red "$*"; exit 1; }

UPSTREAM_URL="${UPSTREAM_URL:-https://github.com/telegramdesktop/tdesktop.git}"
UPSTREAM_BRANCH="${UPSTREAM_BRANCH:-dev}"
MODE="${1:-merge}"  # merge | rebase | fetch-only

command -v git >/dev/null || die "git required"

if ! git remote get-url upstream >/dev/null 2>&1; then
  yellow "Adding upstream remote: $UPSTREAM_URL"
  git remote add upstream "$UPSTREAM_URL"
fi

yellow "Fetching upstream..."
git fetch upstream --tags

case "$MODE" in
  fetch-only)
    green "Fetched upstream/$UPSTREAM_BRANCH ($(git rev-parse --short upstream/$UPSTREAM_BRANCH 2>/dev/null || echo '?'))"
    git log --oneline HEAD..upstream/$UPSTREAM_BRANCH 2>/dev/null | head -20 || true
    exit 0
    ;;
  rebase)
    yellow "Rebasing onto upstream/$UPSTREAM_BRANCH..."
    git rebase "upstream/$UPSTREAM_BRANCH" || die "Rebase conflicts — resolve, then: git rebase --continue"
    ;;
  merge|*)
    yellow "Merging upstream/$UPSTREAM_BRANCH..."
    git merge "upstream/$UPSTREAM_BRANCH" -m "merge: upstream tdesktop $UPSTREAM_BRANCH for security/protocol updates" \
      || die "Merge conflicts — resolve files, then: git add -A && git commit"
    ;;
esac

yellow "Updating submodules..."
git submodule update --init --recursive || yellow "submodule warnings"

if [[ -f scripts/security-audit.sh ]]; then
  bash scripts/security-audit.sh || yellow "Audit reported issues — fix before release"
fi

green "Upstream sync step done. Rebuild with scripts/build-*.sh and test."
