// Drakeygram — offline translate path
// NO external server contact during translation.
// Network for packs happens only in OfflineMtPacks::refreshCatalog / downloadPack (user action).

#include "ayu/features/translator/implementations/google.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"

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
	// Intentionally no QNetworkAccessManager usage here — zero server contact while translating.
	static_assert(OfflineMtPacks::kNeverUploadUserText);
	static_assert(OfflineMtPacks::kNoBackgroundNetwork);

	const auto &text = args.parsedData.text;
	auto fromLang = args.parsedData.fromLang.trimmed();
	const auto toLang = args.parsedData.toLang.trimmed();
	const auto onFail = args.onFail;

	if (text.text.trimmed().isEmpty() || toLang.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}
	if (fromLang.isEmpty()) fromLang = QStringLiteral("en");

	if (!OfflineMtPacks::instance().isInstalled(fromLang, toLang)) {
		LOG(("OfflineMT: pack not on disk — download from Settings first (only then uses network)"));
		if (onFail) onFail();
		return nullptr;
	}

	// Local inference only (Bergamot when linked). No server.
	LOG(("OfflineMT: local pack present; no network during translate"));
	if (onFail) onFail();
	return nullptr;
}

}
