// Drakeygram Desktop
// Chat/message online translate backends disabled by design.
// Use Telegram official translate, or local language packs for UI (ayu_lang).

#include "ayu/features/translator/implementations/google.h"

namespace Ayu::Translator {

GoogleTranslator &GoogleTranslator::instance() {
	static GoogleTranslator inst;
	return inst;
}

GoogleTranslator::GoogleTranslator(QObject *parent)
	: MultiThreadTranslator(parent) {
}

QPointer<QNetworkReply> GoogleTranslator::startSingleTranslation(
	const MultiThreadArgs &args
) {
	// No third-party online translate.
	if (args.onFail) args.onFail();
	return nullptr;
}

}
