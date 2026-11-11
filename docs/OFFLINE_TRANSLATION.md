# Offline translation — Option 1 (lightweight)

## Design

| Piece | Source | When |
|-------|--------|------|
| **Engine** | Shipped **with the app** under `offline-engine/` | Install/update app |
| **Language packs** | Official Mozilla / Firefox models | User downloads in Settings |

No runtime engine download. No terminal setup for users. Chat text never uploaded.

## Why not link full Bergamot into Telegram?

Full Marian/Bergamot **in-process** needs MKL/OpenBLAS and a large binary (build ~30+ minutes). That fights a **lightweight** client.

Lightweight Option 1 = ship a **small stripped CLI** next to the app and run it only when translating offline. Models stay on-demand.

## Maintainer (once per release)

```bash
# build or obtain a Release bergamot-translator, then:
export DRAKEY_OFFLINE_ENGINE_BIN=/path/to/bergamot-translator
bash scripts/bundle-offline-engine.sh /path/to/app/install/prefix
```

App layout:

```text
Drakeygram
offline-engine/bergamot-translator   # or .exe on Windows
```

## User flow

1. Install Drakeygram (engine already inside the package)
2. Settings → Offline (bundled engine)
3. Download only the language packs you need
4. Translate — local only

## Status in Settings

Shows **Offline engine: ready (bundled)** when the binary is present next to the app.
