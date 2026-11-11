# Session protection (Drakeygram)

## What you asked for

| Goal | How |
|------|-----|
| Sessions stay alive across app updates | Keep `tdata/`; update vault under `tdata/drakey/session_guard/update_vault/` |
| Strong local storage protection | Telegram **local passcode** (encrypts account keys) + **device-bound seal** |
| Stolen session files hard to use elsewhere | Seal MAC requires **this machine fingerprint + your secret** |
| Unlimited / persistent login | Local sessions persist until **you** log out or **Telegram server** revokes them |

## Layers

1. **Telegram Desktop local passcode** (built-in) — encrypts local key material in `tdata`. **Enable it.**
2. **SessionGuard seal** (`ayu/security/session_guard.*`) — HMAC bound to device + secret.
3. **Update vault** — copies seal so upgrades do not drop binding markers.

## Anti-theft reality

| Attacker has | Result with protection enabled |
|--------------|--------------------------------|
| Only copied `tdata` on another PC | Seal verification **fails** (different device fingerprint) |
| `tdata` + your local secret on another PC | Still fails device binding (fingerprint differs) |
| Your unlocked PC + secret while logged in | Can use the live session (physical access) |
| Telegram server force-logout | Session dies everywhere (server authority) |

No client can promise “stolen data never works” against a full compromise of an **unlocked** running device. This stack makes **offline theft of session files** far less useful.

## Enable

1. Settings → enable **Local passcode** (Telegram Desktop).
2. Enable Drakeygram session protection (calls `SessionGuard::createOrRefreshSeal` with that secret).
3. Before updating the app: do **not** delete `tdata`; vault keeps seal markers.

## Update app safely

```bash
# Never delete the profile / tdata directory when installing a new build.
# Optional explicit vault refresh is done by SessionGuard::writeUpdateVault().
```

## CMake

Add to `Telegram/CMakeLists.txt` near other `ayu/` sources:

```cmake
        ayu/security/session_guard.cpp
        ayu/security/session_guard.h
```
