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

## Build scripts
* `scripts/build-linux.sh`, `scripts/build-macos.sh`, `scripts/build-windows.ps1`:
  one-command setup and build with progress output and logs. Syntax-checked
  only (bash -n, shellcheck); not run end to end. The PowerShell script has not
  been parsed or run at all.

## Experimental options (Settings -> Experimental)
* "Hide similar channels": hides the similar-channels suggestions under the
  "you joined this channel" message. Off by default. (untested)
* "Don't send read receipts" (data_histories.cpp): chats are marked read only
  locally; "Mark as read" actions also stay local. (untested)
* "Don't send typing status" (api_send_progress.cpp). (untested)
* "Stay offline" (api_updates.cpp): never reports the account as online. (untested)
* "Don't mark stories as viewed" (data_stories.cpp). (untested)
* "Hide sponsored messages" (sponsored_messages.cpp). (untested)
* "No notification delay" (notifications_manager.cpp): notifications show
  without the extra wait. (untested)
* "Streamer mode" (window_controller.cpp): hides app windows from screenshots
  and screen recording, using the existing screenshot-protection service.
  Restart after changing. (untested)
* "Hide folder counters" (chat_filters_tabs_slider.cpp, window_filters_menu.cpp):
  no unread counters on folder tabs or the folders sidebar. Restart after
  changing. (untested)
* "Confirm before sending" (history_widget.cpp) and "Hide similar channels
  tab" (info_profile_tab_peer_lists.cpp). (untested)
All of these are off by default and are not allowed by Telegram's API terms in
published apps; see README.md.
* "Keep deleted messages" (data_session.cpp): text messages that other people
  delete stay on screen with a trash marker until the app restarts. Messages
  with media, service messages and your own messages are not kept. (untested)
* "Hide All chats folder button" (window_filters_menu.cpp): hides the button
  in the folders sidebar only; the top tab strip is unchanged. (untested)
* "Message filters" (history_view_element.cpp): hides incoming text messages
  containing a phrase listed in message-filters.txt in the app data folder.
  Hidden messages still exist; this only changes display. (untested)
