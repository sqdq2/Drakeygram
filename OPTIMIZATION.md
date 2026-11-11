# Drakeygram — safe optimization charter

**Rule:** Optimize without removing or weakening any feature, privacy control, or safety guarantee.

## Non‑negotiables (never “optimize away”)

| Guarantee | Why |
|-----------|-----|
| Telegram protocol / multi-session | Core product |
| No AyuGram trackers / RC phone-home | Privacy |
| No Google/Yandex/dpaste for fork features | Privacy |
| Offline packs = official Mozilla GET only | Trust + ToS hygiene |
| Chat text never uploaded on offline path | Privacy |
| SessionGuard + local passcode path | Anti-theft |
| Filters local-only after network kill | Privacy |
| Upstream sync path | Future Telegram features |

## What we optimize

### 1. Build (speed + binary size, features intact)

- `CMAKE_BUILD_TYPE=Release`
- Parallel compile: `cmake --build . -j$(nproc)`
- Optional LTO only when `DRAKEY_ENABLE_LTO=1` (test before release)
- Strip symbols on packaged CLI engine only (`strip`), not by disabling features
- Bundle offline engine as **external slim CLI** next to app (Option 1 lightweight) — avoids multi‑hundred‑MB in-process Marian link

### 2. Runtime offline MT

- Engine process started only when translating offline
- Packs downloaded only for languages the user chooses
- No background pack refresh unless user taps refresh
- Decode timeout capped (120s) so a stuck engine cannot hang the UI forever

### 3. Network surface

- Telegram: as upstream
- Pack servers: user-initiated GET only
- Engine: **no** runtime download (bundled)
- Security audit fails the build on forbidden sinks

### 4. Code health

- Prefer changes under `ayu/` to reduce upstream merge cost
- After every upstream merge: `apply-all-cmake-extras.sh` + `security-audit.sh` + `production-ready.sh`

## Explicitly rejected “optimizations”

- Removing offline MT, SessionGuard, or filters to shrink code
- Re-enabling cloud translate sinks for “speed”
- Skipping security-audit in CI/release
- Shipping debug builds as release
- Disabling multi-account or local passcode

## Release checklist (full features)

```bash
git checkout dev && git pull
bash scripts/apply-all-cmake-extras.sh
bash scripts/security-audit.sh
bash scripts/production-ready.sh
# package with engine:
#   DRAKEY_OFFLINE_ENGINE_BIN=... bash scripts/bundle-offline-engine.sh "$PREFIX"
./scripts/build-linux.sh   # or macOS / Windows
```

## Feature matrix pointer

See `docs/FEATURE_MATRIX.md` — update it when features change; never delete a row to hide a regression.
