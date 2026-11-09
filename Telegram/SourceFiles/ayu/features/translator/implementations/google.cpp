// Drakeygram Desktop
// Offline MT path: NO local LibreTranslate server (no 127.0.0.1:5000).
// Language packs are downloaded from official Mozilla model hosting into tdata.
// Full neural inference requires Bergamot runtime (see docs/OFFLINE_TRANSLATION.md).
// Until Bergamot is linked, this backend fails closed (no cloud fallback).

#include "ayu/features/translator/implementations/google.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Ayu::Translator {
namespace {

QString modelsRoot() {
	return cWorkingDir() + u"tdata/drakey/mt_models/"_q;
}

bool packInstalled(const QString &src, const QString &trg) {
	const auto dir = modelsRoot() + src + QStringLiteral("-") + trg;
	return QDir(dir).exists() && !QDir(dir).isEmpty();
}

} // namespace

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

	if (text.text.trimmed().isEmpty() || toLang.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}
	if (fromLang.isEmpty()) {
		fromLang = QStringLiteral("en");
	}

	// Official packs must be downloaded first (Settings → Offline packs).
	if (!packInstalled(fromLang, toLang) && !packInstalled(QStringLiteral("en"), toLang)) {
		LOG(("Offline MT: pack not installed for %1→%2. Download from official catalog first.")
			.arg(fromLang, toLang));
		if (onFail) onFail();
		return nullptr;
	}

	// Neural decode happens only when Bergamot (or equivalent) is linked.
	// Fail closed — do not call cloud Google/Yandex or any remote translate API.
	LOG(("Offline MT: models present for %1→%2 but in-process Bergamot engine not linked yet.")
		.arg(fromLang, toLang));
	if (onFail) onFail();
	return nullptr;
}

}
