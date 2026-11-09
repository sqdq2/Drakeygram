<div align="center">

# Drakeygram

**Independent Telegram Desktop client — privacy first, production-oriented**

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](https://github.com/sqdq2/Drakeygram)
[![License](https://img.shields.io/badge/license-GPL--3.0-green)](https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL)
[![Build](https://img.shields.io/badge/build-scripts%20included-success)](https://github.com/sqdq2/Drakeygram/tree/dev/scripts)

</div>

---

## Quick build (all platforms)

Get **API ID** + **API Hash**: [my.telegram.org](https://my.telegram.org)

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
```

| OS | Command |
|----|---------|
| **Linux** | `chmod +x scripts/build-linux.sh && ./scripts/build-linux.sh` |
| **macOS** | `chmod +x scripts/build-macos.sh && ./scripts/build-macos.sh` |
| **Windows** | In VS x64 Developer PowerShell: `powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1` |

Scripts **only ask for API ID and Hash**, then configure + build.  
Optional env: `TELEGRAM_API_ID`, `TELEGRAM_API_HASH`, `BUILD_TYPE=Release`, `JOBS=8`, `OUT_DIR=./out`, `CMAKE_EXTRA_ARGS=...`

Upstream dependency guides:

- [Linux](https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-linux.md)
- [macOS](https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-mac.md)
- [Windows](https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-win-x64.md)

Binary name: **Drakeygram** · Linux desktop id: `com.drakeygram.desktop`

---

## Features

| Area | Features |
|------|----------|
| Privacy | Ghost mode (read/online/typing/stories/upload), silent send |
| Messages | Anti-recall, edit history, local regex filters |
| UX | Fonts, streamer mode, local Premium look, message shot |
| Languages | Official Telegram UI languages + **Settings → General → Language packs** (download Arabic, Russian, … offline) |
| Translate | **Telegram** official only (no Google/Yandex cloud) |

---

## Network policy

| Allowed | Blocked |
|---------|---------|
| Telegram DCs / CDN | AyuGram / exteragram / remote config |
| Optional language JSON from **this repo** `lang/` | dpaste, iTunes, Google/Yandex translate |

---

## Scripts in this repo

```text
scripts/build-linux.sh      # Linux all-in-one
scripts/build-macos.sh      # macOS all-in-one
scripts/build-windows.ps1   # Windows all-in-one
lang/index.json             # Language pack catalog
lang/*.json                 # Downloadable packs
```

---

## License

GPL-3.0 family (Telegram Desktop). Not affiliated with Telegram FZ-LLC.

**Drakeygram** — independent, private, yours.
