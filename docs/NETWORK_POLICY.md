# When Drakeygram contacts external servers (fork features)

## Telegram core

Messaging, media, official langpacks, official Telegram translate → **Telegram servers only** (normal desktop client).

## Fork-added features (`ayu/`)

| Action | Network? |
|--------|----------|
| App start / idle | **No** extra servers |
| Translate a message (Offline provider) | **No** — local disk only |
| User taps **Refresh catalog** | **Yes** — HTTPS GET catalog |
| User taps **Download language/model pack** | **Yes** — HTTPS GET model file |
| RCManager / AyuGram config | **Never** |
| Google / Yandex translate APIs | **Never** |
| dpaste / filter publish | **Never** |

Pack servers are contacted **only** for pack download / catalog refresh initiated by the user.
