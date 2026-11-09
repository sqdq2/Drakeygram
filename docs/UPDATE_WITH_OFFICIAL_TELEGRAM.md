# Update Drakeygram with official Telegram Desktop

Official security and protocol fixes ship in:
https://github.com/telegramdesktop/tdesktop

## Automatic

```bash
cd Drakeygram
bash scripts/sync-upstream-telegram.sh          # merge upstream/dev
# or:
bash scripts/sync-upstream-telegram.sh rebase
bash scripts/sync-upstream-telegram.sh fetch-only
```

Then:

```bash
bash scripts/security-audit.sh
./scripts/build-linux.sh   # or macOS / Windows script
```

## Manual

```bash
git remote add upstream https://github.com/telegramdesktop/tdesktop.git
git fetch upstream
git merge upstream/dev
git submodule update --init --recursive
```

Resolve conflicts, rebuild, test login + messaging + ghost mode.

## Keep after every merge

- `Telegram/SourceFiles/ayu/` privacy and feature code
- No reintroduction of AyuGram / Google / Yandex / dpaste sinks
- Run `scripts/security-audit.sh`
