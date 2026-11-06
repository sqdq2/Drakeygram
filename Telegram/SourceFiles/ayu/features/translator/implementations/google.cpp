// Drakeygram Desktop
// Fully independent fork. No external tracking.
// Google Translate network backend disabled.

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
	// Intentionally no network. External Google Translate API removed.
	if (args.onFail) {
		args.onFail();
	}
	return nullptr;
}

}
