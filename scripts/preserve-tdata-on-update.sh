#!/usr/bin/env bash
# Remind / check that tdata is preserved across upgrades.
set -euo pipefail
echo "Drakeygram: sessions live in the profile tdata directory."
echo "When updating the app binary, DO NOT delete tdata / user data folders."
echo "SessionGuard vault:: tdata/drakey/session_guard/update_vault/"
echo "If you move machines, re-create the seal on the new device after passcode unlock."
