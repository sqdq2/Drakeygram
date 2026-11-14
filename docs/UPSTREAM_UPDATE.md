# Update Drakeygram from official Telegram Desktop

## Latest official (checked)

- **Release:** [v7.4.4](https://github.com/telegramdesktop/tdesktop/releases/tag/v7.4.4) (published 2026-11-13)
- **Dev branch:** `telegramdesktop/tdesktop` → `dev`

## Recommended (on your machine)

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
git checkout dev
git pull origin dev

# Sync latest stable release tag
FORCE_UPSTREAM_TAG=v7.4.4 bash scripts/sync-upstream-telegram.sh merge

# Or track upstream development branch:
# UPSTREAM_REF=upstream/dev bash scripts/sync-upstream-telegram.sh merge

# If merge conflicts: fix files, then
#   git add -A && git commit
#   bash scripts/apply-all-cmake-extras.sh
#   bash scripts/production-ready.sh

git status
# commit CMakeLists changes if the rename script modified them
git add Telegram/CMakeLists.txt scripts/ 2>/dev/null || true
git commit -m "merge: tdesktop v7.4.4 + restore Drakeygram cmake extras" || true
git push origin dev

bash scripts/production-ready.sh   # must show GATE PASSED
./scripts/build-linux.sh           # or macOS / Windows
```

## What the sync script does

1. Adds/fetches `upstream` = `https://github.com/telegramdesktop/tdesktop.git`
2. Merges tag or `upstream/dev` into your branch
3. Re-runs `apply-all-cmake-extras.sh` (offline translator, session_guard, offline rename)
4. Runs `production-ready.sh` gate

## Conflict tips

| Area | Advice |
|------|--------|
| `Telegram/SourceFiles/ayu/**` | Prefer **ours** (Drakeygram) unless upstream deleted the tree |
| `Telegram/CMakeLists.txt` | Merge both; then re-run `apply-all-cmake-extras.sh` |
| Core Telegram UI / MTProto | Prefer **upstream** for security fixes |

## Do not ship until

1. `bash scripts/production-ready.sh` → **GATE PASSED**
2. Full compile succeeds
