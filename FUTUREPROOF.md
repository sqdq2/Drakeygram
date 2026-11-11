# Future-proofing Drakeygram

## Principles

1. **Upstream first for protocol** — merge tdesktop regularly (`scripts/sync-upstream-telegram.sh`).
2. **Isolate fork code** under `Telegram/SourceFiles/ayu/` so merges stay manageable.
3. **Never regress privacy** — audit must stay green (`scripts/security-audit.sh`).
4. **Optional features fail closed** — offline MT / seals do not phone home on idle.
5. **Document matrix** — update `docs/FEATURE_MATRIX.md` when you add/break a feature.

## Release ritual

```bash
git checkout dev
bash scripts/sync-upstream-telegram.sh fetch-only   # review
# merge when ready:
bash scripts/sync-upstream-telegram.sh
bash scripts/apply-all-cmake-extras.sh
bash scripts/production-ready.sh
./scripts/build-linux.sh   # or mac/win
bash scripts/security-audit.sh
```

## Known gaps (track until closed)

| Gap | Impact | Direction |
|-----|--------|-----------|
| `plugin_info_box.cpp` stub on remote | Build/UI risk | Restore full file |
| `filters_utils` dpaste on remote | Privacy regress | Clipboard-only publish |
| CMake missing offline_mt + session_guard until scripts run | Link errors | `apply-all-cmake-extras.sh` |
| Bergamot not linked | Offline chat MT decode incomplete | Native dependency later |
| SessionGuard not hooked into every startup path | Enable explicitly | Wire verify after passcode |

## Compatibility

- Telegram multi-device sessions: unchanged
- App updates: preserve `tdata`
- New official Telegram features: arrive via upstream merge
