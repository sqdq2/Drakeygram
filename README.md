# Drakeygram 

Drakeygram is an **unofficial, independent** desktop client for the
[Telegram][telegram] messaging service. It is **not** made, endorsed or
supported by Telegram FZ-LLC, and it is not affiliated with any other
third-party Telegram client.

Drakeygram uses the [Telegram API][telegram_api] and is part of the Telegram
ecosystem, as required by the [Telegram API Terms of Service][tos].

## Origin and license

Drakeygram is a modified copy of the open-source
[Telegram Desktop][tdesktop] source code. It is published under the
**GNU GPLv3 with the OpenSSL exception**. The license is in [LICENSE](LICENSE)
and the original notices are in [LEGAL](LEGAL). If you distribute binaries of
this project you must also make the corresponding source code available under
the same license.

## Telegram API compliance

This project follows the [Telegram API Terms of Service][tos]:

* Every build must use its **own `api_id` and `api_hash`**, obtained at
  <https://my.telegram.org>. See [docs/api_credentials.md](docs/api_credentials.md).
  Do not ship builds that use the test credentials.
* The name does not use "Telegram" and the official Telegram logo is not used.
* Some **optional, off-by-default** switches in Settings -> Experimental go
  against the terms and are meant for personal builds only, never for
  published ones: don't send read receipts, don't send typing status, stay
  offline, don't mark stories as viewed, and hide sponsored messages. Using
  them can get an `api_id` revoked or an account limited. They affect only
  what you send or see yourself.
* The built-in updater is disabled by default, because it downloads official
  Telegram Desktop packages.

## One-command build

Each script installs what it needs, clones this repository with its
submodules, builds, and prints step-by-step progress (`[3/7] ...` with elapsed
time; the compiler's own `[done/total]` counter is shown during compilation).
A full log is written to `build-logs/`. You need your own API credentials
(see above); the script asks for them if `TDESKTOP_API_ID` and
`TDESKTOP_API_HASH` are not set, and never writes them to disk.

| System | Command |
| --- | --- |
| Linux (needs Docker, installs it if you agree) | `curl -fsSL https://raw.githubusercontent.com/sqdq2/Drakeygram/main/scripts/build-linux.sh \| bash` |
| macOS (needs Xcode from the App Store) | `curl -fsSL https://raw.githubusercontent.com/sqdq2/Drakeygram/main/scripts/build-macos.sh \| bash` |
| Windows (PowerShell) | `irm https://raw.githubusercontent.com/sqdq2/Drakeygram/main/scripts/build-windows.ps1 -OutFile $env:TEMP\dg.ps1; powershell -ExecutionPolicy Bypass -File $env:TEMP\dg.ps1` |

Options: `--debug` (`-Debug` on Windows), `--yes` (`-Yes`), `--dir PATH`
(`-Dir`), and `--test-credentials` (`-TestCredentials`) for a compile check with
Telegram's test-only credentials. The Linux script also reuses cached
libraries published by this repository's CI, which can turn a multi-hour
library build into minutes. These scripts have not been run end to end yet.
Read a script before piping it into a shell.

## Building (manual)

Follow the platform guides in [docs](docs/) (for example
[docs/building-linux.md](docs/building-linux.md)) and pass your own credentials:

```
-D TDESKTOP_API_ID=<your api_id> -D TDESKTOP_API_HASH=<your api_hash>
```

## Status

Work in progress. Changes made on top of the upstream source are listed in
[CHANGES.md](CHANGES.md); anything not listed there is unmodified upstream
behavior. Changes have not yet been verified by a full build.

[telegram]: https://telegram.org
[telegram_api]: https://core.telegram.org
[tos]: https://core.telegram.org/api/terms
[tdesktop]: https://github.com/telegramdesktop/tdesktop
