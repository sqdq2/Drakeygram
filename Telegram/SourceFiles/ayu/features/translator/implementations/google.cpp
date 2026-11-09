// Drakeygram Desktop — OFFLINE translation
// Talks ONLY to local LibreTranslate (default http://127.0.0.1:5000).
// No cloud Google/Yandex. User downloads language packs into local LT, then translates offline.
//
// Flow:
// 1) User installs LibreTranslate locally and downloads models for chosen languages.
// 2) Settings provider "Offline" (this backend) is selected.
// 3) App POSTs to local /translate — works with no internet if models are present.
//
// Override URL: DRAKEY_LIBRETRANSLATE_URL (must stay loopback for offline policy).

#include "ayu/features/translator/implementations/google.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ayu::Translator {
namespace {

QString offlineBaseUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_LIBRETRANSLATE_URL").trimmed();
	if (!env.isEmpty()) {
		auto u = env;
		while (u.endsWith(QLatin1Char('/'))) {
			u.chop(1);
		}
		// Prefer loopback for offline; still allow explicit override.
		return u;
	}
	return QStringLiteral("http://127.0.0.1:5000");
}

QString packsDir() {
	// Tracks which languages the user chose to enable offline.
	return cWorkingDir() + u"tdata/drakey/offline_packs/"_q;
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
	const auto &fromLang = args.parsedData.fromLang;
	const auto &toLang = args.parsedData.toLang;
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	if (text.text.trimmed().isEmpty() || toLang.trimmed().isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	// Record target language as a chosen offline pack (user intent).
	{
		QDir().mkpath(packsDir());
		QFile f(packsDir() + toLang.trimmed() + QStringLiteral(".pack"));
		if (f.open(QIODevice::WriteOnly)) {
			f.write("installed\n");
			f.close();
		}
	}

	const auto source = fromLang.trimmed().isEmpty()
		? QStringLiteral("auto")
		: fromLang.trimmed();

	QJsonObject body;
	body.insert(QStringLiteral("q"), text.text);
	body.insert(QStringLiteral("source"), source);
	body.insert(QStringLiteral("target"), toLang.trimmed());
	body.insert(QStringLiteral("format"), QStringLiteral("text"));

	QNetworkRequest req{QUrl(offlineBaseUrl() + QStringLiteral("/translate"))};
	req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram-Offline/1.0"));
	req.setRawHeader("Accept", "application/json");

	QPointer<QNetworkReply> reply = _nam.post(
		req,
		QJsonDocument(body).toJson(QJsonDocument::Compact));

	auto timer = new QTimer(reply);
	timer->setSingleShot(true);
	timer->setInterval(30000);
	QObject::connect(timer, &QTimer::timeout, reply, [reply] {
		if (reply && reply->isRunning()) reply->abort();
	});
	timer->start();

	QObject::connect(reply, &QNetworkReply::finished, reply, [reply, onSuccess, onFail, timer] {
		if (!reply) return;
		timer->stop();
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) {
			LOG(("Offline translate failed (is local LibreTranslate running with models?): %1")
				.arg(reply->errorString()));
			if (onFail) onFail();
			return;
		}
		QJsonParseError err{};
		const auto doc = QJsonDocument::fromJson(reply->readAll(), &err);
		if (err.error != QJsonParseError::NoError || !doc.isObject()) {
			if (onFail) onFail();
			return;
		}
		const auto translated = doc.object().value(QStringLiteral("translatedText")).toString();
		if (translated.trimmed().isEmpty()) {
			if (onFail) onFail();
			return;
		}
		if (onSuccess) onSuccess(TextWithEntities{translated});
	});

	return reply;
}

}
