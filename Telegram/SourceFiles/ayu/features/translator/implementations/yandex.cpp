// Drakeygram Desktop — production
// Third-party online translate disabled. Use Telegram official translate.

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
	if (args.onFail) {
		args.onFail();
	}
	return nullptr;
}

}
