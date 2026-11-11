# Feature matrix

| Feature | Status |
|---------|--------|
| Telegram messaging / multi-session | Working |
| Ghost mode / local privacy | Working |
| RCManager trackers | Disabled |
| Google/Yandex cloud MT | Disabled |
| Telegram translate | Working |
| Offline pack download (Mozilla GET, user only) | Implemented |
| Offline neural decode via local Bergamot CLI | Implemented (needs binary on machine) |
| SessionGuard device seal | Implemented + init in ayu_infra |
| Sessions survive update | Keep tdata + vault |
| Filters without dpaste | Restore production filters_utils if remote still has dpaste |
| plugin_info_box full | Restore if remote still stub |
| Build/audit/upstream scripts | Present |

## Pending only if environment incomplete

1. Bergamot **binary** installed for offline decode to return text
2. CMake extras applied: `bash scripts/apply-all-cmake-extras.sh`
3. Restore stub/dpaste files if gate fails
