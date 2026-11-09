# Offline Bergamot translation — download only, zero upload

## Privacy rule

| Direction | Allowed |
|-----------|---------|
| **Download** language/model packs (HTTP **GET**) | Yes — official Mozilla model host |
| **Upload** chat text / messages / any user content | **Never** |
| Local LibreTranslate server `127.0.0.1` | **Not used** |
| Google / Yandex cloud translate | **Not used** |

Chat text is only processed **on device**. Network is used solely to **fetch model files** the user chooses.

## Official download source

- Catalog: `lang/mt_index.json` (this repo) or `DRAKEY_MT_CATALOG_URL`
- Model bytes: Mozilla Firefox Translations  
  `https://media.githubusercontent.com/media/mozilla/firefox-translations-models/main/models`

## User flow

1. Settings → Offline MT → refresh catalog  
2. Download **only** the language pairs you need (e.g. `en-ar`)  
3. Packs stored in `tdata/drakey/mt_models/<id>/`  
4. Translate messages / whole chat locally when Bergamot runtime is linked  

## Code

- `Telegram/SourceFiles/ayu/features/translator/offline/offline_mt_packs.*` — GET catalog + GET models only  
- Offline provider never attaches message text to a network request  

## Bergamot runtime

In-process decoding uses [bergamot-translator](https://github.com/browsermt/bergamot-translator) (Mozilla/Browser MT).  
Linking that library into the Telegram Desktop CMake graph is the remaining native step so downloaded packs actually decode text offline.

Until it is linked: packs still **download only**; translate fails closed **without uploading** anything.
