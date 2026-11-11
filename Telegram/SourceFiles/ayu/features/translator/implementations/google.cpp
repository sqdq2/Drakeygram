// Offline provider — Option 1: bundled engine + Mozilla packs. No engine network.

#include "ayu/features/translator/implementations/google.h"
#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"

#include <QtCore/QTimer>
#include <QtWidgets/QApplication>

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
	static_assert(OfflineMtPacks::kNeverUploadUserText);

	const auto text = args.parsedData.text;
	auto fromLang = args.parsedData.fromLang.trimmed();
	const auto toLang = args.parsedData.toLang.trimmed();
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	if (text.text.trimmed().isEmpty() || toLang.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}
	if (fromLang.isEmpty()) {
		fromLang = QStringLiteral("en");
	}

	// Local file copy of bundled engine if needed — no download.
	OfflineMtEngine::Instance().ensureBundledEngineReady();

	QTimer::singleShot(0, qApp, [text, fromLang, toLang, onSuccess, onFail] {
		const auto result = OfflineMtEngine::Instance().decode(
			text.text, fromLang, toLang);
		if (!result.ok) {
			LOG(("OfflineMT: %1").arg(result.error));
			if (onFail) onFail();
			return;
		}
		if (onSuccess) {
			onSuccess(TextWithEntities{ result.text });
		}
	});

	return nullptr;
}

}
