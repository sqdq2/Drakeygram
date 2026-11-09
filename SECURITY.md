# Drakeygram security controls

## Objectives

- No third-party tracking endpoints
- No upload of chat text for translation or analytics
- Model/language packs: **HTTPS GET only**
- Stay mergeable with official Telegram Desktop for protocol/security fixes

## Network policy (enforced in fork code)

| Direction | Rule |
|-----------|------|
| Telegram DCs | Allowed (core client) |
| Pack/catalog download | **HTTPS GET only** |
| Chat text to Google/Yandex/LibreTranslate cloud | **Forbidden** |
| AyuGram / exteragram / remote config | **Forbidden** |
| dpaste / arbitrary filter URL publish | **Forbidden** |

## Local data

- Settings and caches under user profile / `tdata`
- Offline models: `tdata/drakey/mt_models/` (downloaded artifacts only)
- No pack uploader in client

## Build / release

- Build with official toolchain; do not embed API secrets in public forks
- API ID/Hash from [my.telegram.org](https://my.telegram.org) at build time only
- Prefer reproducible Release builds (`scripts/build-*.sh`)

## Upstream Telegram security updates

Official protocol and crypto fixes land in [telegramdesktop/tdesktop](https://github.com/telegramdesktop/tdesktop).

```bash
bash scripts/sync-upstream-telegram.sh
```

See script help for merge/rebase modes.

## Audit

```bash
bash scripts/security-audit.sh
```

Must exit 0 before release.

## Reporting

Report vulnerabilities privately to the repository owner. Do not file public issues with exploit details.
