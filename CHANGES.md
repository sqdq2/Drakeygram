# Changes relative to upstream Telegram Desktop

Status key: **untested** means the change was written but the project has not
been compiled or run.

## Accounts
* No limit on the number of logged-in accounts (`Main::Domain::kUnlimitedMaxAccounts`);
  the Premium "accounts" row was removed from the Premium promo. (untested)

## Branding and compliance
* App name is "Drakeygram (Unofficial client)"; installer GUID changed. (untested)
* Built-in updater disabled by default. (untested)
* README rewritten with the unofficial-client and API disclosure.
* Original app icon replaces the Telegram logo in `Telegram/Resources/art/icon*`.
