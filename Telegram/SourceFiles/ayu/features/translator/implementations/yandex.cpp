// Drakeygram Desktop
// Fully independent fork. No external tracking.
// Yandex Translate network backend disabled.

#include "ayu/features/translator/implementations/yandex.h"

namespace Ayu::Translator {

YandexTranslator &YandexTranslator::instance() {
	static YandexTranslator inst;
	return inst;
}

YandexTranslator::YandexTranslator(QObject *parent)
	: MultiThreadTranslator(parent) {
}

QSet<QString> YandexTranslator::supportedLanguages() const {
	return {};
}

QPointer<QNetworkReply> YandexTranslator::startSingleTranslation(
	const MultiThreadArgs &args
) {
	// Intentionally no network. External Yandex Translate API removed.
	if (args.onFail) {
		args.onFail();
	}
	return nullptr;
}

}
