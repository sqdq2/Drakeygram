// Drakeygram translator router — Telegram or Offline only.
// Google/Yandex cloud providers are not offered.

#include "ayu/features/translator/ayu_translator.h"
#include "ayu/features/translator/implementations/google.h"
#include "ayu/features/translator/implementations/yandex.h"
#include "ayu/ayu_settings.h"

// NOTE: If this file is shorter than upstream full ayu_translator.cpp on disk,
// keep the rest of the file body from your tree. Only translatorForProvider
// must route as below.

namespace Ayu::Translator {
namespace {

BaseTranslator *translatorForProvider(TranslationProvider provider) {
	switch (provider) {
	case TranslationProvider::Offline:
		return &OfflineTranslator::instance(); // after rename in Step 3
	case TranslationProvider::Telegram:
	case TranslationProvider::Native:
	default:
		return nullptr;
	}
}

} // namespace
}
