<div align="center">

# Drakeygram

**Independent Telegram Desktop client — privacy first**

</div>

## Translation (works in-app, no local server)

| Provider | How |
|----------|-----|
| **Telegram** (default) | Official Telegram translate servers |
| **LibreTranslate** | Public API `https://libretranslate.com` — **no install** |

Optional:

```bash
export DRAKEY_LIBRETRANSLATE_URL=https://libretranslate.com   # default
export DRAKEY_LIBRETRANSLATE_API_KEY=your_key                 # if instance requires it
```

Choose the non-Telegram provider in settings (wired to LibreTranslate). Batch / chat translation uses the built-in multi-message path.

## Language packs (on demand)

1. **Main UI languages** — official **Telegram** langpacks (normal desktop behavior).
2. **Drakeygram extra strings** — downloaded **when needed** from this repo:

   `https://raw.githubusercontent.com/sqdq2/Drakeygram/main/lang/{langId}.json`

   Cached under `tdata/drakey/languages/`. Override base URL with `DRAKEY_LANG_BASE_URL`.

Put packs in the repo `lang/` folder (e.g. `lang/en.json`).

## Network policy

- Telegram servers (required)
- LibreTranslate public API (only if you use that provider)
- This GitHub `lang/` JSON (only for extra UI strings)
- **No** AyuGram / Google / Yandex / iTunes / dpaste

## Build scripts (only ask API ID + Hash)

```bash
# Linux
chmod +x scripts/build-linux.sh && ./scripts/build-linux.sh

# macOS
chmod +x scripts/build-macos.sh && ./scripts/build-macos.sh

# Windows (VS Developer PowerShell)
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1
```

Get API ID/Hash: https://my.telegram.org  
Deps: same as [Telegram Desktop](https://github.com/telegramdesktop/tdesktop).

## License

GPL-3.0 family. Not affiliated with Telegram FZ-LLC.
