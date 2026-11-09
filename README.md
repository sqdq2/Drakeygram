<div align="center">

# Drakeygram

**Independent Telegram Desktop client — privacy first, production-oriented**

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](https://github.com/sqdq2/Drakeygram)
[![License](https://img.shields.io/badge/license-GPL--3.0-green)](https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL)
[![Build](https://img.shields.io/badge/build-scripts%20included-success)](https://github.com/sqdq2/Drakeygram/tree/main/scripts)

</div>

## Quick build (all platforms)

API ID + Hash: [my.telegram.org](https://my.telegram.org)

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
```

| OS | Command |
|----|---------|
| **Linux** | `chmod +x scripts/build-linux.sh && ./scripts/build-linux.sh` |
| **macOS** | `chmod +x scripts/build-macos.sh && ./scripts/build-macos.sh` |
| **Windows** | VS x64 Developer PowerShell: `powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1` |

Scripts only ask for **API ID** and **Hash**. Optional: `TELEGRAM_API_ID`, `TELEGRAM_API_HASH`, `BUILD_TYPE`, `JOBS`, `OUT_DIR`.

## Features

Ghost mode · Anti-recall · Filters · Streamer mode · Language packs (Settings → General) · Telegram translate only

## Network

Telegram only (+ optional `lang/` packs from this repo). No AyuGram / Google / Yandex / dpaste.

## License

GPL-3.0 family. Not affiliated with Telegram FZ-LLC.
