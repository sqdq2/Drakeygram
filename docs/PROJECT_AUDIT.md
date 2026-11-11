# Drakeygram project audit (dev)

## Component status

| Component | Status | Notes |
|-----------|--------|-------|
| OfflineTranslator (`offline.cpp/h`) | Present | Local only |
| google/yandex sources | **Removed** | Must stay gone |
| `ayu_translator.cpp` | Present (~9k+) | Routes Offline |
| `offline_mt_engine` | Present | Bundled engine path |
| `offline_mt_packs` | Present | Mozilla GET only |
| RCManager | Local-only | No network |
| SessionGuard | Present | |
| filters_utils | Present (full) | No dpaste |
| plugin_info_box | Present (full) | |
| CMake offline_mt + session_guard | Present | |
| CMake offline.cpp | **Must apply rename** | `apply-cmake-offline-rename.sh` auto-runs in gate |

## Forbidden sinks (ayu sources)

Scanned clean for: dpaste.com, Google Translate hosts, Yandex translate hosts, ayugram.one/org.

## Error-handling map

| Scenario | Handling |
|----------|----------|
| Missing API ID/Hash | Build script exits with message |
| Non-numeric API ID | Rejected |
| Short API hash | Rejected |
| cmake not installed | Exit 1 before configure |
| cmake configure fail | Non-zero exit + hint (deps/submodules) |
| compile fail | Non-zero exit + CMake rename hint |
| Truncated critical sources | production-ready dies on size |
| Stale google/yandex files | production-ready dies |
| CMake lists google/yandex | Auto-rename then re-check |
| Forbidden network sink | Gate + security-audit fail |
| Offline pack missing | Decode returns error; UI fail callback |
| Engine not bundled | Clear error string; no crash |
| Engine timeout | Kill process; error result |
| Empty translate input | Fail callback |
| RCManager | No requests ever |

## Optimize without feature loss

- Release `-j` parallel builds
- Optional LTO only via `DRAKEY_ENABLE_LTO=1`
- Engine shipped beside app (not in-process Marian)
- Packs on-demand only
- Gate runs before every scripted build

## Commands

```bash
git pull origin dev
bash scripts/apply-all-cmake-extras.sh
bash scripts/production-ready.sh
./scripts/build-linux.sh
```
