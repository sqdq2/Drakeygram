// Drakeygram Desktop
// Fully independent fork. No external tracking.
// LibreTranslate — works out of the box via public HTTPS API (no local server).
// Override: DRAKEY_LIBRETRANSLATE_URL, DRAKEY_LIBRETRANSLATE_API_KEY

#include "ayu/features/translator/implementations/google.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ayu::Translator {
namespace {

// Public LibreTranslate-compatible API (no install required).
// Users can override with self-hosted URL via env if they prefer.
QString libreTranslateBaseUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_LIBRETRANSLATE_URL");
	if (!env.trimmed().isEmpty()) {
		auto u = env.trimmed();
		while (u.endsWith(QLatin1Char('/'))) {
			u.chop(1);
		}
		return u;
	}
	return QStringLiteral("https://libretranslate.com");
}

QString libreTranslateApiKey() {
	return qEnvironmentVariable("DRAKEY_LIBRETRANSLATE_API_KEY").trimmed();
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
		if (onFail) {
			onFail();
		}
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
	const auto apiKey = libreTranslateApiKey();
	if (!apiKey.isEmpty()) {
		body.insert(QStringLiteral("api_key"), apiKey);
	}

	QNetworkRequest req{QUrl(libreTranslateBaseUrl() + QStringLiteral("/translate"))};
	req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram/1.0"));
	req.setRawHeader("Accept", "application/json");

	QPointer<QNetworkReply> reply = _nam.post(
		req,
		QJsonDocument(body).toJson(QJsonDocument::Compact));

	auto timer = new QTimer(reply);
	timer->setSingleShot(true);
	timer->setInterval(25000);
	QObject::connect(timer, &QTimer::timeout, reply, [reply] {
		if (reply && reply->isRunning()) {
			reply->abort();
		}
	});
	timer->start();

	QObject::connect(reply, &QNetworkReply::finished, reply, [reply, onSuccess, onFail, timer] {
		if (!reply) {
			return;
		}
		timer->stop();
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) {
			LOG(("LibreTranslate error: %1").arg(reply->errorString()));
			if (onFail) {
				onFail();
			}
			return;
		}
		QJsonParseError err{};
		const auto doc = QJsonDocument::fromJson(reply->readAll(), &err);
		if (err.error != QJsonParseError::NoError || !doc.isObject()) {
			if (onFail) {
				onFail();
			}
			return;
		}
		const auto obj = doc.object();
		auto translated = obj.value(QStringLiteral("translatedText")).toString();
		if (translated.trimmed().isEmpty() && obj.value(QStringLiteral("translatedText")).isArray()) {
			// some instances return array
		}
		if (translated.trimmed().isEmpty()) {
			if (onFail) {
				onFail();
			}
			return;
		}
		if (onSuccess) {
			onSuccess(TextWithEntities{translated});
		}
	});

	return reply;
}

}
