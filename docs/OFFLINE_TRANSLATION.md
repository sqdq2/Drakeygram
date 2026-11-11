# Offline translation — fully in-app

No terminal scripts or environment variables required for normal use.

## User flow

1. Settings → **Offline (in-app)**
2. Tap **Install offline engine** (one-time HTTPS download into `tdata/drakey/engine/`)
3. Download language packs you need (Mozilla models, HTTPS GET)
4. Translate chats — runs locally; chat text is never uploaded

## What the app stores

| Path | Content |
|------|--------|
| `tdata/drakey/engine/` | Offline engine binary |
| `tdata/drakey/mt_models/` | Language/model packs |

## Engine release assets

The app downloads from:

```text
https://github.com/sqdq2/Drakeygram/releases/download/engine-v1/bergamot-translator-<platform>
```

Publish those assets under GitHub Release tag **`engine-v1`** (linux-x64 / macos / windows-x64.exe).

Until the release exists, install shows an error toast; place a binary manually at `tdata/drakey/engine/bergamot-translator` if needed.

## Privacy

- Engine + packs: GET only when user installs/downloads
- No Google/Yandex/LibreTranslate cloud for offline path
- No local server
