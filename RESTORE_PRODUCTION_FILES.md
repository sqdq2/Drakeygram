# Restore production files (required)

Two files on `dev` may be incomplete after automated pushes. Replace them from this guide.

## 1. plugin_info_box.cpp

Path: `Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp`

```bash
# From a good historical revision then remove pypi:
curl -sL 'https://raw.githubusercontent.com/sqdq2/Drakeygram/ec2c67f6f9eeb0fa0e3f0683fc9abe6b07693fdd/Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp' \
  -o Telegram/SourceFiles/ayu/ui/boxes/plugin_info_box.cpp
```

Then replace the pypi callback with:

```cpp
chip->setClickedCallback([dep] {
    Q_UNUSED(dep);
});
```

## 2. filters_utils.cpp (remove dpaste)

Path: `Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp`

Ensure `importFromLink` and `publishFilters` do **not** call the network:
- `importFromLink` → toast fail only
- `publishFilters` → clipboard export only (no dpaste.com)

## Verify

```bash
grep -RniE 'dpaste|pypi.org|translate.googleapis|translate.yandex|ayugram\.one' Telegram/SourceFiles/ayu || echo clean
```
