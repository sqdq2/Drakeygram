# Drakeygram production readiness

## Status (automated audit)

| Area | Status |
|------|--------|
| RCManager / AyuGram remote config | Disabled |
| Google / Yandex translate APIs | Disabled (Telegram only) |
| dpaste / arbitrary filter URL import | Must be disabled on build branch |
| pypi.org outbound clicks | Must be removed |
| Branding Drakeygram | CMake + xdg |
| Build scripts Linux/macOS/Windows | `scripts/build-*.sh` / `.ps1` |
| Language packs catalog | `lang/index.json` |
| Settings → Language packs UI | `settings_general.cpp` on dev |

## Build (production)

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
git checkout dev
chmod +x scripts/build-linux.sh   # or build-macos.sh
./scripts/build-linux.sh          # prompts API ID + Hash only
```

Windows: VS x64 Developer PowerShell → `scripts\build-windows.ps1`

## Network policy

Allowed: Telegram servers; optional `lang/*.json` from this repository.  
Blocked: AyuGram, exteragram, Google/Yandex translate, dpaste, iTunes, pypi auto-open.

## Merge policy

Default branch: **dev**. Resolve conflicts by keeping full `settings_general.cpp` (Language packs + all General toggles). Never commit truncated settings/filters files.

## Pre-release checklist

- [ ] Clean build on target OS
- [ ] Login + send message
- [ ] Ghost mode toggles
- [ ] Settings → General → Language packs download
- [ ] No outbound to ayugram / googleapis translate / yandex / dpaste (optional network monitor)
- [ ] Binary named Drakeygram
