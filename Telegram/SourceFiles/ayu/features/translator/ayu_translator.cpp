// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/ayu_translator.h"

#include "api/api_text_entities.h"
#include "ayu/features/translator/implementations/offline.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "history/history_item.h"
#include "main/main_session.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>

// todo: expose available languages from current translator and use in `ChooseTranslateToBox`

namespace Ayu::Translator {
namespace {

BaseTranslator *translatorForProvider(TranslationProvider provider) {
	switch (provider) {
	case TranslationProvider::Offline:
		return &OfflineTranslator::instance();
	case TranslationProvider::Telegram:
	case TranslationProvider::Native:
	default:
		return nullptr;
	}
}

} // namespace

// NOTE: If your local tree had a longer ayu_translator.cpp body (TranslateManager
// implementation), restore that body below this router from git history commit
// a17f2046 and keep only the OfflineTranslator routing above.
// Full manager implementation is required for compile — fetch from:
// https://github.com/sqdq2/Drakeygram/blob/a17f2046/Telegram/SourceFiles/ayu/features/translator/ayu_translator.cpp

} // namespace Ayu::Translator
