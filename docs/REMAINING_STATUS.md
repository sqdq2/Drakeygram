# Remaining status (auto-checked)

| Item | Live `dev` | How to finish |
|------|------------|---------------|
| SessionGuard in `ayu_infra` | **Done** | — |
| Offline MT packs/engine sources | In tree | CMake list |
| CMake lists offline_mt + session_guard | Not until script | `bash scripts/apply-all-cmake-extras.sh` |
| filters dpaste | Still on remote | `bash scripts/fix-remaining.sh` then push |
| plugin_info_box stub | Still on remote | same script then push |
| Bergamot binary | Local machine | `DRAKEY_BERGAMOT_BIN` |

```bash
git pull origin dev
bash scripts/fix-remaining.sh
git add -A && git commit -m "fix: filters + plugin + cmake" && git push
bash scripts/production-ready.sh
./scripts/build-linux.sh
```
