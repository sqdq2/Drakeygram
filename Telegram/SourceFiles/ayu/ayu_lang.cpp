// Drakeygram Desktop
// Language packs downloaded on demand into local cache, then used offline.
// Catalog: lang/index.json from official project source (this repository).
// Core Telegram UI languages still use official Telegram langpack (MTProto).

#include "ayu/ayu_lang.h"

#include "qjsondocument.h"
#include "core/application.h"
#include "lang/lang_instance.h"
#include "storage/localstorage.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

namespace {

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

QString languagePackBaseUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_LANG_BASE_URL");
	if (!env.trimmed().isEmpty()) {
		auto u = env.trimmed();
		while (u.endsWith(QLatin1Char('/'))) u.chop(1);
		return u;
	}
	return QStringLiteral(
		"https://raw.githubusercontent.com/sqdq2/Drakeygram/main/lang");
}

} // namespace

AyuLanguage *AyuLanguage::instance = nullptr;

AyuLanguage::AyuLanguage() {
	loadCatalogFromDisk();
}

void AyuLanguage::init() {
	if (!instance) instance = new AyuLanguage;
	instance->loadCachedLanguage();
	// Discover new languages / updates (catalog only — small JSON).
	instance->refreshCatalog();
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

QString AyuLanguage::getCatalogPath() const {
	return getCacheDir() + u"index.json"_q;
}

void AyuLanguage::loadCatalogFromDisk() {
	_catalog.clear();
	QFile file(getCatalogPath());
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}
	QJsonParseError err{};
	const auto doc = QJsonDocument::fromJson(file.readAll(), &err);
	file.close();
	if (err.error != QJsonParseError::NoError || !doc.isObject()) {
		return;
	}
	const auto arr = doc.object().value(QStringLiteral("languages")).toArray();
	for (const auto &v : arr) {
		const auto o = v.toObject();
		DrakeyLanguageInfo info;
		info.id = o.value(QStringLiteral("id")).toString();
		info.name = o.value(QStringLiteral("name")).toString();
		info.nativeName = o.value(QStringLiteral("native")).toString();
		if (info.id.isEmpty()) continue;
		info.installed = QFile::exists(getCachePath(info.id));
		_catalog.push_back(std::move(info));
	}
}

void AyuLanguage::saveCatalogToDisk(const QByteArray &json) {
	QDir().mkpath(getCacheDir());
	QFile file(getCatalogPath());
	if (file.open(QIODevice::WriteOnly)) {
		file.write(json);
		file.close();
	}
}

std::vector<DrakeyLanguageInfo> AyuLanguage::availableLanguages() const {
	return _catalog;
}

QStringList AyuLanguage::installedLanguageIds() const {
	QStringList out;
	const auto dir = QDir(getCacheDir());
	const auto files = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
	for (const auto &f : files) {
		if (f == QStringLiteral("index.json")) continue;
		out.push_back(QFileInfo(f).completeBaseName());
	}
	return out;
}

bool AyuLanguage::isInstalled(const QString &id) const {
	return QFile::exists(getCachePath(id));
}

void AyuLanguage::refreshCatalog() {
	const auto url = languagePackBaseUrl() + QStringLiteral("/index.json");
	LOG(("Drakeygram: refreshing language catalog %1").arg(url));
	QNetworkRequest req{QUrl(url)};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram/1.0"));
	if (_catalogReply) {
		_catalogReply->abort();
		_catalogReply->deleteLater();
	}
	_pending = PendingKind::Catalog;
	_catalogReply = networkManager.get(req);
	connect(_catalogReply, &QNetworkReply::finished, this, &AyuLanguage::catalogFinished);
	connect(_catalogReply, &QNetworkReply::errorOccurred, this, &AyuLanguage::catalogError);
}

void AyuLanguage::catalogFinished() {
	if (!_catalogReply) return;
	const auto reply = _catalogReply;
	_catalogReply = nullptr;
	reply->deleteLater();
	_pending = PendingKind::None;
	if (reply->error() != QNetworkReply::NoError) {
		LOG(("Drakeygram: catalog download failed"));
		return;
	}
	const auto data = reply->readAll();
	QJsonParseError err{};
	const auto doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError || !doc.isObject()) {
		return;
	}
	saveCatalogToDisk(data);
	loadCatalogFromDisk();
	emit catalogUpdated();
	LOG(("Drakeygram: language catalog updated (%1 languages)").arg(_catalog.size()));
}

void AyuLanguage::catalogError(QNetworkReply::NetworkError e) {
	Q_UNUSED(e);
	if (_catalogReply) {
		_catalogReply->deleteLater();
		_catalogReply = nullptr;
	}
	_pending = PendingKind::None;
}

void AyuLanguage::loadCachedLanguage() {
	const auto langPackId = Lang::GetInstance().id();
	const auto langPackBaseId = Lang::GetInstance().baseId();
	auto finalLangPackId = langMapping.contains(langPackId) ? langMapping[langPackId] : langPackId;
	if (finalLangPackId.isEmpty()) finalLangPackId = langPackBaseId;
	if (finalLangPackId.isEmpty()) return;
	useLocalLanguage(finalLangPackId);
}

void AyuLanguage::useLocalLanguage(const QString &id) {
	if (id.isEmpty()) return;
	QFile file(getCachePath(id));
	if (!file.exists()) return;
	if (!file.open(QIODevice::ReadOnly)) return;
	const auto data = file.readAll();
	file.close();
	QJsonParseError error{};
	const auto doc = QJsonDocument::fromJson(data, &error);
	if (error.error == QJsonParseError::NoError) {
		LOG(("Using local language pack: %1").arg(id));
		applyLanguageJson(doc);
	}
}

void AyuLanguage::saveCachedLanguage(const QByteArray &json, const QString &langId) {
	QDir().mkpath(getCacheDir());
	QFile file(getCachePath(langId));
	if (file.open(QIODevice::WriteOnly)) {
		file.write(json);
		file.close();
	}
}

void AyuLanguage::fetchLanguage(const QString &id, const QString &baseId) {
	auto finalLangPackId = langMapping.contains(id) ? langMapping[id] : id;
	_currentLangId = finalLangPackId.isEmpty() ? baseId : finalLangPackId;
	if (_currentLangId.isEmpty()) return;

	// Already local → use immediately (offline).
	if (isInstalled(_currentLangId)) {
		useLocalLanguage(_currentLangId);
	}

	const auto url = languagePackBaseUrl() + QLatin1Char('/') + _currentLangId + QStringLiteral(".json");
	LOG(("Drakeygram: downloading language pack %1").arg(url));
	QNetworkRequest req{QUrl(url)};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram/1.0"));
	if (_chkReply) {
		_chkReply->abort();
		_chkReply->deleteLater();
	}
	_pending = PendingKind::Pack;
	_chkReply = networkManager.get(req);
	connect(_chkReply, &QNetworkReply::finished, this, &AyuLanguage::fetchFinished);
	connect(_chkReply, &QNetworkReply::errorOccurred, this, &AyuLanguage::fetchError);
}

void AyuLanguage::fetchFinished() {
	if (!_chkReply) return;
	const auto reply = _chkReply;
	_chkReply = nullptr;
	reply->deleteLater();
	_pending = PendingKind::None;
	if (reply->error() != QNetworkReply::NoError) {
		emit languageDownloadFailed(_currentLangId, reply->errorString());
		return;
	}
	const auto data = reply->readAll();
	QJsonParseError error{};
	const auto doc = QJsonDocument::fromJson(data, &error);
	if (error.error != QJsonParseError::NoError || !doc.isObject()) {
		emit languageDownloadFailed(_currentLangId, QStringLiteral("invalid JSON"));
		return;
	}
	saveCachedLanguage(data, _currentLangId);
	applyLanguageJson(doc);
	// refresh installed flags
	for (auto &info : _catalog) {
		if (info.id == _currentLangId) info.installed = true;
	}
	emit languageDownloaded(_currentLangId);
	emit catalogUpdated();
}

void AyuLanguage::fetchError(QNetworkReply::NetworkError e) {
	Q_UNUSED(e);
	const auto err = _chkReply ? _chkReply->errorString() : QStringLiteral("network error");
	if (_chkReply) {
		_chkReply->deleteLater();
		_chkReply = nullptr;
	}
	_pending = PendingKind::None;
	emit languageDownloadFailed(_currentLangId, err);
}

void AyuLanguage::applyLanguageJson(QJsonDocument doc) {
	const auto json = doc.object();
	for (const QString &brokenKey : json.keys()) {
		auto key = qsl("ayu_") + brokenKey;
		auto val = json.value(brokenKey).toString().replace(qsl("&amp;"), qsl("&"));
		if (key.endsWith("_Android")) continue;
		for (const auto &postfix : postfixes) {
			if (key.endsWith(qsl("_") + postfix)) {
				key = key.replace(qsl("_") + postfix, qsl("#") + postfix);
				break;
			}
		}
		if (key.endsWith("_PC")) key = key.replace("_PC", "");
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
