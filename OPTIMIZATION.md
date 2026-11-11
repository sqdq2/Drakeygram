# Safe optimization charter

## Permanently removed (not merely “disabled”)

| Removed | Status |
|---------|--------|
| AyuGram / remote config trackers | RCManager local-only; no requests |
| Google Translate API | No endpoints; Offline path only |
| Yandex Translate API | Hard-removed; UI migrates off Yandex |
| dpaste publish / URL filter import | Removed from filters_utils |

## Still present (features)

- Telegram official translate
- Offline local packs (Mozilla) + bundled engine
- SessionGuard, filters (local), full messaging

Do not reintroduce removed sinks for speed or convenience.
