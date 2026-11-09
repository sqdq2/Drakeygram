<div align="center">

# Drakeygram

**Independent Telegram Desktop client — production scripts on `dev`**

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](https://github.com/sqdq2/Drakeygram)
[![License](https://img.shields.io/badge/license-GPL--3.0-green)](https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL)

</div>

## Production build

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram && git checkout dev
chmod +x scripts/*.sh
bash scripts/production-ready.sh    # gate: sizes, security, cmake
./scripts/build-linux.sh            # or build-macos.sh / build-windows.ps1
```

API ID + Hash: [my.telegram.org](https://my.telegram.org)

Full guide: **[PRODUCTION_BUILD.md](PRODUCTION_BUILD.md)**

| Script | Purpose |
|--------|--------|
| `scripts/production-ready.sh` | Pre-build gate |
| `scripts/build-linux.sh` | Linux Release build |
| `scripts/build-macos.sh` | macOS Release build |
| `scripts/build-windows.ps1` | Windows Release build |
| `scripts/sync-upstream-telegram.sh` | Merge official Telegram Desktop |
| `scripts/security-audit.sh` | Forbidden endpoint scan |

## Security

- No AyuGram remote config / trackers
- No Google/Yandex cloud translate
- Offline MT packs: **HTTPS GET only** — never uploads chat text
- Filters: no dpaste publish
- Details: [SECURITY.md](SECURITY.md)

## Update from official Telegram

```bash
bash scripts/sync-upstream-telegram.sh
bash scripts/security-audit.sh
# rebuild
```

## License

GPL-3.0 family. Not affiliated with Telegram FZ-LLC.
