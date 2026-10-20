# Drakeygram (Unofficial client)

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
* Sponsored messages are shown as the official client shows them and are not
  interfered with.
* The following are deliberately **not** implemented, because the terms forbid
  them: hiding read, online/last-seen or typing status ("ghost mode"),
  preventing self-destructing content from disappearing, unlocking paid
  Telegram Premium features locally, and bypassing a chat's copy or download
  restrictions.
* The built-in updater is disabled by default, because it downloads official
  Telegram Desktop packages.

## Building

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
