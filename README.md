<div align="center">

# Drakeygram

**Independent Telegram Desktop client — privacy first, Telegram network only (+ optional self-hosted LibreTranslate)**

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](https://github.com/sqdq2/Drakeygram)
[![License](https://img.shields.io/badge/license-GPL--3.0-green)](https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL)

</div>

---

## Features

- **Ghost Mode** — read receipts, online, typing, stories, upload progress
- **Anti-recall** & edited message history (local)
- Regex filters, streamer mode, fonts, message shot
- **Translation**
  - **Telegram** official translate (default, Telegram servers only)
  - **LibreTranslate** (optional) — self-hosted by default at `http://127.0.0.1:5000`; supports batch / chat translation paths already in the client
- Language packs: Telegram official UI + optional Drakeygram JSON from **this repo** (`/lang`)

## Network policy

| Allowed | Blocked |
|--------|--------|
| Telegram DCs / CDN | AyuGram / exteragram / Radolyn |
| Self-hosted LibreTranslate (opt-in) | Google Translate, Yandex, iTunes, dpaste |
| This repo language JSON (opt-in) | Arbitrary filter URL import |

Set `DRAKEY_LIBRETRANSLATE_URL` to point at your LibreTranslate instance.  
Set `DRAKEY_LANG_BASE_URL` to override language JSON base (defaults to this repo’s `/lang`).

---

## Build (all-in-one scripts)

Get **API ID** and **API Hash** from [my.telegram.org](https://my.telegram.org).

### Linux

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
chmod +x scripts/build-linux.sh
./scripts/build-linux.sh
# prompts for API ID + Hash if not set in env
```

Env optional: `TELEGRAM_API_ID`, `TELEGRAM_API_HASH`, `BUILD_TYPE=Release`, `JOBS=8`, `CMAKE_EXTRA_ARGS=...`

### macOS

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
chmod +x scripts/build-macos.sh
./scripts/build-macos.sh
```

### Windows (PowerShell)

Open **x64 Native Tools** / VS Developer PowerShell, then:

```powershell
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1
```

Scripts only ask for **API ID** and **API Hash**, then configure CMake and build target `Telegram` (output name **Drakeygram**).

Dependencies follow official Telegram Desktop docs:

- [Linux](https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-linux.md)
- [macOS](https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-mac.md)
- [Windows](https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-win-x64.md)

---

## LibreTranslate (optional chat translation)

1. Run LibreTranslate locally, e.g. Docker:
   ```bash
   docker run -p 5000:5000 libretranslate/libretranslate
   ```
2. In Drakeygram, choose the non-Telegram translation provider (Google slot → LibreTranslate backend).
3. Or: `export DRAKEY_LIBRETRANSLATE_URL=http://127.0.0.1:5000`

Batch / multi-message translation uses the existing provider pipeline (entire chat selection supported by the UI batch path).

Default remains **Telegram** translate (no extra servers).

---

## Restore helpers (if a large file was truncated on GitHub)

- `RESTORE_CMAKE.sh` — restore full `Telegram/CMakeLists.txt` + Drakeygram names  
- `scripts/mass_rebrand_headers.py` — strip old AyuGram file headers  
- `RESTORE_FILTERS_UTILS.txt` — notes for `filters_utils.cpp`

---

## License

GPL-3.0 family (Telegram Desktop). Not affiliated with Telegram FZ-LLC.

**Drakeygram** — independent, private, yours.
