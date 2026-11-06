// Drakeygram Desktop
// Fully independent fork. No external tracking.
// LibreTranslate backend (replaces Google). Configurable endpoint.
// Default: http://127.0.0.1:5000 (self-hosted). Set DRAKEY_LIBRETRANSLATE_URL to override.

#include "ayu/features/translator/implementations/google.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ayu::Translator {
namespace {

QString libreTranslateBaseUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_LIBRETRANSLATE_URL");
	if (!env.trimmed().isEmpty()) {
		return env.trimmed().replace(QRegularExpression(QStringLiteral("/+")), QStringLiteral("/")).remove(QRegularExpression(QStringLiteral("/$")));
	}
	// Local self-hosted LibreTranslate by default (no third-party cloud).
	return QStringLiteral("http://127.0.0.1:5000");
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

	const auto source = fromLang.trimmed().isEmpty()
		? QStringLiteral("auto")
		: fromLang.trimmed();
	const auto target = toLang.trimmed();

	QJsonObject body;
	body.insert(QStringLiteral("q"), text.text);
	body.insert(QStringLiteral("source"), source);
	body.insert(QStringLiteral("target"), target);
	body.insert(QStringLiteral("format"), QStringLiteral("text"));

	const auto url = libreTranslateBaseUrl() + QStringLiteral("/translate");
	QNetworkRequest req{QUrl(url)};
	req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram/1.0"));

	QPointer<QNetworkReply> reply = _nam.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));

	auto timer = new QTimer(reply);
	timer->setSingleShot(true);
	timer->setInterval(20000);
	QObject::connect(timer, &QTimer::timeout, reply, [reply] {
		if (reply && reply->isRunning()) reply->abort();
	});
	timer->start();

	QObject::connect(reply, &QNetworkReply::finished, reply, [reply, onSuccess, onFail, timer] {
		if (!reply) return;
		timer->stop();
		const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(
			reply, [](QNetworkReply *r) { r->deleteLater(); });
		if (reply->error() != QNetworkReply::NoError) {
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
