# Feature matrix (do not drop rows to hide regressions)

| Feature | Status | Notes |
|---------|--------|-------|
| Telegram messaging | Working | Upstream |
| Multi-account / multi-device sessions | Working | Keep `tdata` on update |
| Ghost mode / local privacy | Working | `ayu/` |
| Anti-recall / local history | Working | Local |
| Filters (local) | Working | No dpaste / no URL import |
| Plugin info UI | Working | Restored full file |
| RCManager trackers | Disabled | Must stay disabled |
| Google/Yandex cloud MT | Disabled | Must stay disabled |
| Telegram translate | Working | Cloud Telegram |
| Offline pack download (Mozilla) | Working | User GET only |
| Offline decode (bundled engine) | Working when packaged | Option 1 lightweight |
| SessionGuard | Working | Init in `ayu_infra` |
| Local passcode | Working | Upstream Desktop |
| Build scripts L/M/W | Present | API ID+Hash |
| Security audit / production gate | Present | Required before ship |
| Upstream sync | Present | `sync-upstream-telegram.sh` |

## Optimization policy

See [OPTIMIZATION.md](../OPTIMIZATION.md): improve speed/size/safety **without** removing anything in this table.
