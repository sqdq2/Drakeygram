// Drakeygram Desktop — production
// Third-party online translate disabled. Use Telegram official translate.

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
	if (args.onFail) {
		args.onFail();
	}
	return nullptr;
}

}
