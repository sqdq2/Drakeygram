# Drakeygram language packs (local after download)

## How it works

1. App downloads **`index.json`** (catalog) → list of languages for the dropdown.
2. User picks a language (Arabic, Russian, …) → app downloads `{id}.json` once.
3. Pack is stored under `tdata/drakey/languages/` and used **offline**.
4. On startup, catalog can refresh to discover **new** languages / updates.

## Files

| File | Purpose |
|------|--------|
| `index.json` | Catalog (id, English name, native name) |
| `{id}.json` | String map for Drakeygram extra UI keys |

## Main Telegram UI languages

Changing the **whole app** language (menus, chats, etc.) still uses **official Telegram** language packs via Settings → Language (Telegram servers). That is separate and already built into Telegram Desktop.

This folder is for **extra Drakeygram** strings (ghost mode labels, etc.).

## Adding a language

1. Add entry to `index.json`.
2. Add `xx.json` with key/value strings.
3. Users get it after catalog refresh + download.
