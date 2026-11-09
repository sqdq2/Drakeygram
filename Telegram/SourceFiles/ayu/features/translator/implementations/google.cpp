// Drakeygram Desktop — Bergamot-oriented offline path
// PRIVACY: Never uploads chat text. No Google/Yandex cloud. No local translate server.
// Only local disk models (downloaded via OfflineMtPacks GET) may be used for inference.
// Full neural decode requires Bergamot linked in the binary (see docs/OFFLINE_TRANSLATION.md).

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
	const auto &text = args.parsedData.text;
	auto fromLang = args.parsedData.fromLang.trimmed();
	const auto toLang = args.parsedData.toLang.trimmed();
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	// Hard guarantee: this function must not open any network request with `text`.
	// OfflineMtPacks may only GET model files; it never receives message bodies.
	static_assert(OfflineMtPacks::kNeverUploadUserText);

	if (text.text.trimmed().isEmpty() || toLang.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}
	if (fromLang.isEmpty()) {
		fromLang = QStringLiteral("en");
	}

	auto &packs = OfflineMtPacks::instance();
	if (!packs.isInstalled(fromLang, toLang)) {
		LOG(("OfflineMT: pack not installed for %1→%2 — download official pack first (GET only)")
			.arg(fromLang, toLang));
		if (onFail) onFail();
		return nullptr;
	}

	// Local-only inference placeholder: Bergamot C++ API hooks here.
	// When bergamot-translator is linked, load packDir models and decode `text` in-process.
	// Until then fail closed — still zero upload of user bytes.
	LOG(("OfflineMT: pack present; Bergamot in-process runtime not linked — no network fallback"));
	if (onFail) onFail();
	return nullptr; // no QNetworkReply involving user text
}

}
