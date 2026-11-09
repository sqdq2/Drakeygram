# Complete production build

## Branch

Use **`dev`** (default).

## 0. Restore truncated files (if gate fails size checks)

| File | Min size | Action |
|------|----------|--------|
| `Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp` | ~15KB | Must be full UI source, no pypi open |
| `Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp` | ~23KB | `importFromLink` / `publishFilters` = no network |

If `plugin_info_box.cpp` is only a few hundred bytes, replace it with the full file from a known-good commit and remove any `pypi.org` `OpenUrl`.

## 1. One command gate

```bash
git clone --recursive https://github.com/sqdq2/Drakeygram.git
cd Drakeygram
git checkout dev
chmod +x scripts/*.sh
bash scripts/production-ready.sh
```

## 2. Build

```bash
# Linux
./scripts/build-linux.sh

# macOS
./scripts/build-macos.sh

# Windows (VS x64 Developer PowerShell)
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1
```

Only prompts: **API ID** + **API Hash** → https://my.telegram.org

## 3. Upstream Telegram updates

```bash
bash scripts/sync-upstream-telegram.sh
bash scripts/security-audit.sh
# rebuild
```

## 4. Release checklist

- [ ] `production-ready.sh` passes
- [ ] `security-audit.sh` passes
- [ ] Release binary runs, login works
- [ ] Ghost mode toggles
- [ ] No AyuGram / Google / Yandex / dpaste in audit
- [ ] Offline packs = HTTPS GET only (no chat upload)

## Scripts

| Script | Role |
|--------|------|
| `production-ready.sh` | Size checks + sinks + cmake + audit |
| `apply-cmake-offline-mt.sh` | Add offline_mt_packs to CMakeLists |
| `security-audit.sh` | Forbidden URL scan |
| `sync-upstream-telegram.sh` | Merge official tdesktop |
| `build-linux.sh` / `build-macos.sh` / `build-windows.ps1` | Full build |
