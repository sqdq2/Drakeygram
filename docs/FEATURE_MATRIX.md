# Drakeygram feature matrix (maintain this when merging upstream)

| Feature | Status | Notes |
|---------|--------|-------|
| Messaging (Telegram) | Working | Upstream tdesktop |
| Ghost mode / privacy toggles | Working | Local `ayu/` |
| Anti-recall / local history | Working | Local |
| Filters (local regex) | Working if filters_utils restored | No dpaste |
| RCManager / AyuGram trackers | Disabled | Must stay disabled after upstream merge |
| Google/Yandex cloud translate | Disabled | Must stay disabled |
| Telegram official translate | Working | Cloud Telegram |
| Offline MT pack download (Mozilla GET) | Code present | HTTPS GET only on user action |
| Offline MT in-process Bergamot decode | Not linked | Future: link bergamot-translator |
| UI language packs catalog | Partial | `lang/index.json` |
| SessionGuard device seal | Code present | Add to CMake; enable + local passcode |
| Sessions across devices | Working | Normal Telegram multi-session |
| Sessions across app update | Working | Keep `tdata/`; vault helpers |
| Build scripts L/M/W | Present | API ID+Hash only |
| Upstream sync script | Present | `scripts/sync-upstream-telegram.sh` |
| Security audit script | Present | Run before every release |

## Future-proof rules

1. After every `sync-upstream-telegram.sh`, run `security-audit.sh` and re-apply CMake helpers.
2. Never re-enable AyuGram remote config, Google/Yandex translate, or dpaste.
3. Pack network = user-initiated HTTPS GET only.
4. Session protection = local passcode + SessionGuard; cannot override Telegram server logout.
5. Prefer additive files under `ayu/` so upstream merges conflict less.
