# Offline translation (optional Bergamot decode)

## Two pieces

| Piece | Required? | What |
|-------|-----------|------|
| Language/model **packs** | For offline MT | Download in app (Mozilla HTTPS GET, user only) |
| **Bergamot** binary | Optional until you want local decode | Runs models on-device |

Without the binary: packs can still download; translate fails closed **locally** (no cloud upload).

## Quick setup (optional)

```bash
# from repo root
bash scripts/setup-bergamot.sh

# if you built/installed bergamot-translator yourself:
export DRAKEY_BERGAMOT_BIN=/path/to/bergamot-translator
# optional when using scripts/bergamot-wrapper.sh:
export DRAKEY_BERGAMOT_REAL=/path/to/real-engine
```

Or place the binary at:

```text
DrakeygramBergamot/bergamot-translator
```

next to the app working directory (auto-discovered).

## CLI contract

```text
$DRAKEY_BERGAMOT_BIN --model-dir <dir> --from <src> --to <trg> --input <file>
# translation on stdout
```

`scripts/bergamot-wrapper.sh` adapts slightly different Bergamot CLIs to this shape.

## In the app

1. Settings → translation provider → **Offline**
2. Download the language pairs you need
3. Translate messages / chats (local decode if binary present)

## Privacy

- No LibreTranslate server
- No Google/Yandex for this path
- Network only for **pack download** when you ask
- Decode never opens the network
