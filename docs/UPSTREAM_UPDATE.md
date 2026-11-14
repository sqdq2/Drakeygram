# Update from official Telegram Desktop

## Important

https://github.com/sqdq2/Drakeygram is **not** a GitHub fork of `telegramdesktop/tdesktop` (no shared parent history).

So this **fails**:

```bash
git merge v7.4.4   # unrelated histories → 1000+ conflicts
```

Use the **overlay** script instead.

## Latest official

- **v7.4.4** — https://github.com/telegramdesktop/tdesktop/releases/tag/v7.4.4

## Do this on your machine

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
git checkout dev
git pull origin dev

# Requires: git, rsync
bash scripts/update-from-tdesktop.sh v7.4.4

git status
git add -A
git commit -m "update: overlay official tdesktop v7.4.4"
git push origin dev

bash scripts/production-ready.sh   # GATE PASSED
./scripts/build-linux.sh           # or macOS / Windows
```

## What overlay does

1. Clones official tag into `.upstream-tdesktop`
2. Copies upstream `Telegram/` + `cmake/` (does **not** delete your `ayu/`)
3. Restores `Telegram/SourceFiles/ayu` and `scripts/`
4. Runs `apply-all-cmake-extras.sh` + `production-ready.sh`

## Alternative (git merge)

Only if you later rewrite history to share commits with tdesktop. Not recommended for this repo layout.
