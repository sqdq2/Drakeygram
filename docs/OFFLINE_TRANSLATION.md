# Offline chat translation (no local server)

## Your requirement

- Download **any language pack you choose** (not all bundled in the app)
- **Entire chat** translation works after packs are installed
- **No** `127.0.0.1` / Docker LibreTranslate server
- Packs come from an **official download endpoint**

## Official model source

Machine-translation packs use **Mozilla Firefox Translations** models (same family as Firefox offline translate):

- Registry: https://github.com/mozilla/firefox-translations-models
- Media base: `https://media.githubusercontent.com/media/mozilla/firefox-translations-models/main/models`
- App catalog: `lang/mt_index.json` in this repo (lists pairs; files are pulled from Mozilla)

## In the app

1. Settings → translation provider → **Offline**
2. Download language pack(s) you need (e.g. English→Arabic)
3. Packs store under `tdata/drakey/mt_models/`
4. Translate message or whole chat (batch path already in the client)

## Engine note (important)

Downloading packs stores **official model files** on disk.  
Running neural translation **fully inside** Telegram Desktop still requires the **Bergamot** engine (Mozilla’s C++ MT runtime) linked into the binary.

| Piece | Status |
|--------|--------|
| Official pack catalog + download UI | Implemented (catalog + paths) |
| Entire-chat batch API | Already in translator pipeline |
| No LibreTranslate localhost server | Yes — removed from design |
| In-process Bergamot inference | Requires adding `bergamot-translator` as a native dependency (next engineering milestone) |

Until Bergamot is linked, use **Telegram** provider for reliable full-chat translate, and use Offline after packs + Bergamot integration land.

## Why not “JSON language packs” alone?

UI JSON packs only change menu text. Chat translation needs **MT models** (tens–hundreds of MB per direction), downloaded on demand from the official host above.
