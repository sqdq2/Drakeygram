// Drakeygram Desktop
// Language packs:
// 1) Core UI — official Telegram langpack (MTProto) via normal desktop client.
// 2) Extra Drakeygram strings — optional JSON from official project source only
//    (this GitHub repo /lang), downloaded on demand and cached locally.

#include "ayu/ayu_lang.h"

#include "qjsondocument.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "lang/lang_instance.h"
#include "storage/localstorage.h"

#include <QDir>
#include <QFile>
#include <QNetworkRequest>
#include <QUrl>

std::map<QString, QString> langMapping = {
	{"pt-br", "pt"},
	{"zh-hans-beta", "zh-hans"},
	{"zh-hant-beta", "zh-hant"},
	{"zh-hans-raw", "zh-hans"},
	{"zh-hant-raw", "zh-hant"},
};

constexpr auto postfixes = {
	"zero", "one", "two", "few", "many", "other"
};

// Official project language packs (user demand / on need).
// Override base with DRAKEY_LANG_BASE_URL if you mirror packs yourself.
QString languagePackBaseUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_LANG_BASE_URL");
	if (!env.trimmed().isEmpty()) {
		auto u = env.trimmed();
		while (u.endsWith(QLatin1Char('/'))) {
			u.chop(1);
		}
		return u;
	}
	// Official source for THIS client only (not AyuGram).
	return QStringLiteral(
		"https://raw.githubusercontent.com/sqdq2/Drakeygram/main/lang");
}

AyuLanguage *AyuLanguage::instance = nullptr;

AyuLanguage::AyuLanguage() = default;

void AyuLanguage::init() {
	if (!instance) {
		instance = new AyuLanguage;
	}
	instance->loadCachedLanguage();
}

AyuLanguage *AyuLanguage::currentInstance() {
	return instance;
}

QString AyuLanguage::getCacheDir() const {
	return cWorkingDir() + u"tdata/drakey/languages/"_q;
}

QString AyuLanguage::getCachePath(const QString &langId) const {
	return getCacheDir() + langId + u".json"_q;
}

void AyuLanguage::loadCachedLanguage() {
	const auto langPackId = Lang::GetInstance().id();
	const auto langPackBaseId = Lang::GetInstance().baseId();
	auto finalLangPackId = langMapping.contains(langPackId)
		? langMapping[langPackId]
		: langPackId;
	if (finalLangPackId.isEmpty()) {
		finalLangPackId = langPackBaseId;
	}
	if (finalLangPackId.isEmpty()) {
		return;
	}

	QFile file(getCachePath(finalLangPackId));
	if (!file.exists()) {
		const auto basePath = getCachePath(langPackBaseId);
		if (!QFile::exists(basePath)) {
			return;
		}
		file.setFileName(basePath);
	}
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}
	const auto data = file.readAll();
	file.close();
	QJsonParseError error{};
	const auto doc = QJsonDocument::fromJson(data, &error);
	if (error.error == QJsonParseError::NoError) {
		LOG(("Loading cached Drakeygram language: %1").arg(finalLangPackId));
		applyLanguageJson(doc);
	}
}

void AyuLanguage::saveCachedLanguage(const QByteArray &json, const QString &langId) {
	QDir().mkpath(getCacheDir());
	QFile file(getCachePath(langId));
	if (file.open(QIODevice::WriteOnly)) {
		file.write(json);
		file.close();
		LOG(("Cached Drakeygram language: %1").arg(langId));
	}
}

void AyuLanguage::fetchLanguage(const QString &id, const QString &baseId) {
	auto finalLangPackId = langMapping.contains(id) ? langMapping[id] : id;
	_currentLangId = finalLangPackId.isEmpty() ? baseId : finalLangPackId;
	if (_currentLangId.isEmpty()) {
		return;
	}

	// Always try cache first (offline / already downloaded).
	if (QFile::exists(getCachePath(_currentLangId))) {
		loadCachedLanguage();
		// Still refresh in background when online.
	}

	const auto url = languagePackBaseUrl()
		+ QLatin1Char('/')
		+ _currentLangId
		+ QStringLiteral(".json");
	LOG(("Drakeygram: downloading language pack on demand: %1").arg(url));

	QNetworkRequest req{QUrl(url)};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram/1.0"));
	req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);

	if (_chkReply) {
		_chkReply->abort();
		_chkReply->deleteLater();
		_chkReply = nullptr;
	}
	_chkReply = networkManager.get(req);
	connect(_chkReply, &QNetworkReply::finished, this, &AyuLanguage::fetchFinished);
	connect(_chkReply, &QNetworkReply::errorOccurred, this, &AyuLanguage::fetchError);
}

void AyuLanguage::fetchFinished() {
	if (!_chkReply) {
		return;
	}
	const auto reply = _chkReply;
	_chkReply = nullptr;
	reply->deleteLater();
	if (reply->error() != QNetworkReply::NoError) {
		LOG(("Drakeygram: language download failed: %1").arg(reply->errorString()));
		// Cache may still have been applied earlier.
		return;
	}
	const auto data = reply->readAll();
	if (data.isEmpty()) {
		return;
	}
	QJsonParseError error{};
	const auto doc = QJsonDocument::fromJson(data, &error);
	if (error.error != QJsonParseError::NoError || !doc.isObject()) {
		LOG(("Drakeygram: language JSON invalid"));
		return;
	}
	saveCachedLanguage(data, _currentLangId);
	applyLanguageJson(doc);
	LOG(("Drakeygram: language pack applied: %1").arg(_currentLangId));
}

void AyuLanguage::fetchError(QNetworkReply::NetworkError e) {
	Q_UNUSED(e);
	if (_chkReply) {
		_chkReply->deleteLater();
		_chkReply = nullptr;
	}
	LOG(("Drakeygram: language pack network error"));
}

void AyuLanguage::applyLanguageJson(QJsonDocument doc) {
	const auto json = doc.object();
	for (const QString &brokenKey : json.keys()) {
		auto key = qsl("ayu_") + brokenKey;
		auto val = json.value(brokenKey).toString().replace(qsl("&amp;"), qsl("&"));
		if (key.endsWith("_Android")) {
			continue;
		}
		for (const auto &postfix : postfixes) {
			if (key.endsWith(qsl("_") + postfix)) {
				key = key.replace(qsl("_") + postfix, qsl("#") + postfix);
				break;
			}
		}
		if (key.endsWith("_PC")) {
			key = key.replace("_PC", "");
		}
		if (val.contains(qsl("%1$d")) && !val.contains(qsl("%2$d"))) {
			val = val.replace(qsl("%1$d"), qsl("{count}"));
		} else if (val.contains(qsl("%1$d")) && val.contains(qsl("%2$d"))) {
			val = val.replace(qsl("%1$d"), qsl("{count1}")).replace(qsl("%2$d"), qsl("{count2}"));
		} else if (val.contains(qsl("%1$s")) && !val.contains(qsl("%2$s"))) {
			val = val.replace(qsl("%1$s"), qsl("{item}"));
		} else if (val.contains(qsl("%1$s")) && val.contains(qsl("%2$s"))) {
			val = val.replace(qsl("%1$s"), qsl("{item1}")).replace(qsl("%2$s"), qsl("{item2}"));
		}
		Lang::GetInstance().resetValue(key.toUtf8());
		Lang::GetInstance().applyValue(key.toUtf8(), val.toUtf8());
	}
	Lang::GetInstance().updatePluralRules();
}
