# Drakeygram

Independent Telegram Desktop client (fork lineage cleaned of AyuGram trackers).

**Privacy-first. Feature-complete. Production scripts included.**

## Highlights

- Full Telegram Desktop capability (messaging, multi-account, multi-device sessions)
- Fork trackers / remote config phone-home **disabled**
- Cloud Google/Yandex translate sinks **disabled**; Telegram translate remains available
- **Offline translation (Option 1 lightweight):** engine ships with the app; language packs from official Mozilla sources on demand; chat text never uploaded
- **SessionGuard:** device-bound local seal + Telegram local passcode recommended
- Filters: local only (no dpaste / no URL import)
- Upstream sync script for official Telegram Desktop updates

## Build (API ID + Hash only)

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
git checkout dev
bash scripts/apply-all-cmake-extras.sh
bash scripts/production-ready.sh

# Linux
./scripts/build-linux.sh

# macOS
./scripts/build-macos.sh

# Windows (PowerShell)
./scripts/build-windows.ps1
```

Scripts prompt for **API ID** and **API Hash** (from https://my.telegram.org).

### Optional offline engine in the package

```bash
export DRAKEY_OFFLINE_ENGINE_BIN=/path/to/stripped/bergamot-translator
bash scripts/bundle-offline-engine.sh /path/to/install/prefix
```

Users do **not** run this — only release packaging.

## Scripts

| Script | Purpose |
|--------|--------|
| `scripts/build-linux.sh` | Linux optimized Release build |
| `scripts/build-macos.sh` | macOS Release build |
| `scripts/build-windows.ps1` | Windows Release build |
| `scripts/production-ready.sh` | Gate: sizes, sinks, cmake extras |
| `scripts/security-audit.sh` | Forbidden network sink scan |
| `scripts/sync-upstream-telegram.sh` | Merge official tdesktop |
| `scripts/apply-all-cmake-extras.sh` | Offline MT + SessionGuard sources |
| `scripts/bundle-offline-engine.sh` | Maintainer: ship engine with app |

## Docs

- [OPTIMIZATION.md](OPTIMIZATION.md) — safe optimization rules (no feature cuts)
- [docs/FEATURE_MATRIX.md](docs/FEATURE_MATRIX.md)
- [docs/OFFLINE_TRANSLATION.md](docs/OFFLINE_TRANSLATION.md)
- [docs/SESSION_PROTECTION.md](docs/SESSION_PROTECTION.md)
- [docs/NETWORK_POLICY.md](docs/NETWORK_POLICY.md)
- [PRODUCTION_BUILD.md](PRODUCTION_BUILD.md)
- [SECURITY.md](SECURITY.md)

## Network policy (short)

1. **Telegram servers** — normal client traffic  
2. **Mozilla pack hosts** — only when the user downloads offline language packs  
3. **Nothing else** for fork features (no AyuGram, dpaste, Google/Yandex translate APIs)

## License

Same family as Telegram Desktop / GPLv3 for corresponding sources. Respect upstream licenses and third-party notices.
