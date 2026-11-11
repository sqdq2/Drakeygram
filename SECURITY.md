# Security / privacy

## Removed

- Third-party trackers / remote config phone-home
- Google Translate API usage
- Yandex Translate API usage
- dpaste filter publish and arbitrary URL filter import

## Allowed network

1. Telegram infrastructure (normal client)
2. Official Mozilla/Firefox model hosts — only when the user downloads offline language packs

## Local protection

- Prefer Desktop **local passcode**
- SessionGuard device seal (see `docs/SESSION_PROTECTION.md`)

Run before every release:

```bash
bash scripts/security-audit.sh
bash scripts/production-ready.sh
```
