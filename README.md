<div align="center">

# Drakeygram

**Independent Telegram Desktop — production build scripts included**

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](https://github.com/sqdq2/Drakeygram)
[![License](https://img.shields.io/badge/license-GPL--3.0-green)](https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL)

</div>

## Production build (API ID + Hash only)

1. Get credentials: [my.telegram.org](https://my.telegram.org)
2. Clone and build:

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
git checkout dev

# Linux
chmod +x scripts/build-linux.sh scripts/apply-cmake-offline-mt.sh
./scripts/build-linux.sh

# macOS
chmod +x scripts/build-macos.sh
./scripts/build-macos.sh

# Windows (VS x64 Developer PowerShell)
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1
```

Optional env: `TELEGRAM_API_ID`, `TELEGRAM_API_HASH`, `BUILD_TYPE=Release`, `JOBS=8`, `OUT_DIR=./out`

Dependencies: same as [Telegram Desktop](https://github.com/telegramdesktop/tdesktop) for your OS.

## Privacy / network

- Telegram servers for messaging
- **No** AyuGram trackers / remote config
- **No** Google/Yandex cloud translate
- Offline MT packs: **download only** (GET models) — **never uploads** chat text
- Filters: no dpaste upload

## Features

Ghost mode · Anti-recall · Filters · Streamer mode · Language packs · Offline MT pack manager (Bergamot path)

See `docs/OFFLINE_TRANSLATION.md` and `PRODUCTION.md`.

## License

GPL-3.0 family. Not affiliated with Telegram FZ-LLC.
