# Offline translation (download only the languages you need)

Drakeygram does **not** ship every language model inside the app (too large).
You download **only the languages you choose**. After that, translation works **offline**.

## How it works

1. Install **LibreTranslate** once on your PC (local engine).
2. Download **language packs (models)** only for languages you need.
3. In Drakeygram: Settings → translation provider → **Offline** (shown as Offline / local).
4. Translate messages as usual — traffic goes to `http://127.0.0.1:5000` only, not the internet.

## Install engine (one time)

### Docker (recommended)

```bash
docker run -d --name libretranslate -p 5000:5000 libretranslate/libretranslate
```

Models download the first time you use a language, or pre-load:

```bash
docker run -d -p 5000:5000 libretranslate/libretranslate --load-only en,ar,ru
```

Change `en,ar,ru` to the languages **you** want.

### Without Docker

Follow: https://github.com/LibreTranslate/LibreTranslate  
Run so it listens on `127.0.0.1:5000`.

## In Drakeygram

1. Start local LibreTranslate (with your chosen language models).
2. Settings → set provider to **Offline** (Google slot = offline backend).
3. Translate any chat/message — works without internet if the local engine has that language pair.

Optional:

```bash
export DRAKEY_LIBRETRANSLATE_URL=http://127.0.0.1:5000
```

## What is NOT offline

- **Telegram** provider = cloud (Telegram servers).
- UI “Language packs” under General = interface text only, not MT models.

## Summary

| Step | Action |
|------|--------|
| Choose languages | e.g. Arabic + Russian only |
| Download packs | LibreTranslate models for those langs |
| App setting | Offline provider |
| Result | Message translation offline |
