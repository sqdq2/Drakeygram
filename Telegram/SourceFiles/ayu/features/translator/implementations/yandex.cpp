// Yandex cloud translate permanently removed. Hard fail only.

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
	// Permanently removed — no Yandex endpoints, no network.
	LOG(("YandexTranslator: removed from Drakeygram"));
	if (args.onFail) {
		args.onFail();
	}
	return nullptr;
}

} // namespace Ayu::Translator
