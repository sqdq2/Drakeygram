# Offline translation (complete path)

## Flow

1. **Download pack** (user action only) → Mozilla models via HTTPS GET → `tdata/drakey/mt_models/<pair>/`
2. **Decode** on device via **Bergamot CLI** (no network, no text upload)
3. Install engine binary once:

```bash
# set path to your bergamot-translator build
export DRAKEY_BERGAMOT_BIN=/path/to/bergamot-translator
```

Or put `bergamot-translator` on `PATH`, or at:
`tdata/../DrakeygramBergamot/bergamot-translator`

## CLI contract expected by Drakeygram

```text
bergamot-translator --model-dir <dir> --from <src> --to <trg> --input <file>
# writes translation to stdout
```

You can wrap the official Bergamot app in a small script that matches this interface.

## Provider

Settings → translation provider → **Offline (local)** uses this path.

## Privacy

- No LibreTranslate server
- No Google/Yandex
- Network only for pack GET when you download
- Decode never opens network
