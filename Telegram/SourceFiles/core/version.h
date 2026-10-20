/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/const_string.h"

#define TDESKTOP_REQUESTED_ALPHA_VERSION (0ULL)

#ifdef TDESKTOP_ALLOW_CLOSED_ALPHA
#define TDESKTOP_ALPHA_VERSION TDESKTOP_REQUESTED_ALPHA_VERSION
#else // TDESKTOP_ALLOW_CLOSED_ALPHA
#define TDESKTOP_ALPHA_VERSION (0ULL)
#endif // TDESKTOP_ALLOW_CLOSED_ALPHA

// used in Updater.cpp and Setup.iss for Windows
constexpr auto AppId = "{0FDC2961-931D-4F82-BA67-4555B52E6099}"_cs;
constexpr auto AppNameOld = "Telegram Win (Unofficial)"_cs;
// Telegram API Terms of Service: an app title that contains "Telegram"
// must be preceded by "Unofficial", so this build does not use that word.
constexpr auto AppName = "Drakeygram (Unofficial client)"_cs;
constexpr auto AppFile = "Telegram"_cs;
constexpr auto AppVersion = 7003001;
constexpr auto AppVersionStr = "7.3.1";
constexpr auto AppBetaVersion = false;
constexpr auto AppAlphaVersion = TDESKTOP_ALPHA_VERSION;
