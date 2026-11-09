// Drakeygram — offline MT pack manager
// CONNECTS TO EXTERNAL SERVERS ONLY WHEN:
//   1) User refreshes pack catalog (refreshCatalog)
//   2) User downloads a language/model pack (downloadPack)
// Never on app start, never during message translate, never in background timers.
// Method: HTTPS GET only. No POST of user/chat data.

#include "ayu/features/translator/offline/offline_mt_packs.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkRequest>

namespace {

QString catalogUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_MT_CATALOG_URL").trimmed();
	if (!env.isEmpty()) return env;
	return QStringLiteral(
		"https://raw.githubusercontent.com/sqdq2/Drakeygram/dev/lang/mt_index.json");
}

QString mozillaModelsBase() {
	const auto env = qEnvironmentVariable("DRAKEY_MT_MODELS_BASE").trimmed();
	if (!env.isEmpty()) {
		auto u = env;
		while (u.endsWith(QLatin1Char('/'))) u.chop(1);
		return u;
	}
	return QStringLiteral(
		"https://media.githubusercontent.com/media/mozilla/firefox-translations-models/main/models");
}

bool isHttpsUrl(const QUrl &url) {
	return url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0;
}

QNetworkRequest makeGetRequest(const QUrl &url) {
	QNetworkRequest req{url};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram-OfflineMT/1.0"));
	req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	return req;
}

} // namespace

OfflineMtPacks &OfflineMtPacks::instance() {
	static OfflineMtPacks inst;
	return inst;
}

OfflineMtPacks::OfflineMtPacks() {
	// LOCAL ONLY — no network at construction / app startup.
	loadLocalCatalog();
}

QString OfflineMtPacks::modelsRoot() const {
	return cWorkingDir() + u"tdata/drakey/mt_models/"_q;
}

QString OfflineMtPacks::packDir(const QString &id) const {
	return modelsRoot() + id + QLatin1Char('/');
}

bool OfflineMtPacks::isInstalled(const QString &src, const QString &trg) const {
	const auto id = src + QStringLiteral("-") + trg;
	return QFile::exists(packDir(id) + QStringLiteral(".installed"));
}

std::vector<OfflineMtPackInfo> OfflineMtPacks::packs() const {
	return _packs;
}

void OfflineMtPacks::loadLocalCatalog() {
	_packs.clear();
	QFile f(modelsRoot() + QStringLiteral("catalog.json"));
	if (!f.open(QIODevice::ReadOnly)) {
		return;
	}
	QJsonParseError err{};
	const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
	f.close();
	if (err.error != QJsonParseError::NoError || !doc.isObject()) {
		return;
	}
	for (const auto &v : doc.object().value(QStringLiteral("languages")).toArray()) {
		const auto o = v.toObject();
		OfflineMtPackInfo info;
		info.id = o.value(QStringLiteral("id")).toString();
		info.name = o.value(QStringLiteral("name")).toString();
		info.src = o.value(QStringLiteral("src")).toString();
		info.trg = o.value(QStringLiteral("trg")).toString();
		if (info.id.isEmpty() && !info.src.isEmpty() && !info.trg.isEmpty()) {
			info.id = info.src + QStringLiteral("-") + info.trg;
		}
		if (info.id.isEmpty()) {
			continue;
		}
		info.installed = QFile::exists(packDir(info.id) + QStringLiteral(".installed"));
		_packs.push_back(std::move(info));
	}
}

void OfflineMtPacks::saveMarker(const QString &id) {
	QDir().mkpath(packDir(id));
	QFile f(packDir(id) + QStringLiteral(".installed"));
	if (f.open(QIODevice::WriteOnly)) {
		f.write("get-only\n");
		f.close();
	}
}

void OfflineMtPacks::refreshCatalog() {
	// USER-INITIATED ONLY — the only catalog network path.
	const auto url = QUrl(catalogUrl());
	if (!isHttpsUrl(url)) {
		LOG(("OfflineMT: catalog URL must be HTTPS"));
		return;
	}
	if (_catalogReply) {
		_catalogReply->abort();
		_catalogReply->deleteLater();
	}
	LOG(("OfflineMT: user requested catalog refresh (HTTPS GET only)"));
	_catalogReply = _nam.get(makeGetRequest(url));
	connect(_catalogReply, &QNetworkReply::finished, this, [this] {
		if (!_catalogReply) return;
		const auto reply = _catalogReply;
		_catalogReply = nullptr;
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) {
			LOG(("OfflineMT: catalog GET failed: %1").arg(reply->errorString()));
			return;
		}
		const auto data = reply->readAll();
		QDir().mkpath(modelsRoot());
		QFile out(modelsRoot() + QStringLiteral("catalog.json"));
		if (out.open(QIODevice::WriteOnly)) {
			out.write(data);
			out.close();
		}
		loadLocalCatalog();
		emit catalogUpdated();
	});
}

void OfflineMtPacks::downloadPack(const QString &id) {
	// USER-INITIATED ONLY — the only model download network path.
	if (id.isEmpty()) {
		emit downloadFinished(id, false, QStringLiteral("empty id"));
		return;
	}
	if (_downloadReply) {
		emit downloadFinished(id, false, QStringLiteral("download in progress"));
		return;
	}
	QString src, trg;
	for (const auto &p : _packs) {
		if (p.id == id) {
			src = p.src;
			trg = p.trg;
			break;
		}
	}
	if (src.isEmpty() || trg.isEmpty()) {
		const auto parts = id.split(QLatin1Char('-'));
		if (parts.size() >= 2) {
			src = parts[0];
			trg = parts[1];
		}
	}
	if (src.isEmpty() || trg.isEmpty()) {
		emit downloadFinished(id, false, QStringLiteral("unknown pack"));
		return;
	}

	const auto fileUrl = QUrl(
		mozillaModelsBase() + QLatin1Char('/') + src + QLatin1Char('-') + trg
		+ QStringLiteral("/model.intgemm.alphas.bin"));
	if (!isHttpsUrl(fileUrl)) {
		emit downloadFinished(id, false, QStringLiteral("HTTPS required"));
		return;
	}

	QDir().mkpath(packDir(id));
	_downloadingId = id;
	LOG(("OfflineMT: user requested pack download (HTTPS GET only): %1").arg(fileUrl.toString()));
	_downloadReply = _nam.get(makeGetRequest(fileUrl));

	connect(_downloadReply, &QNetworkReply::downloadProgress, this,
		[this](qint64 rec, qint64 tot) {
			emit downloadProgress(_downloadingId, rec, tot);
		});

	connect(_downloadReply, &QNetworkReply::finished, this, [this] {
		if (!_downloadReply) return;
		const auto reply = _downloadReply;
		_downloadReply = nullptr;
		const auto id = _downloadingId;
		_downloadingId.clear();
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) {
			emit downloadFinished(id, false, reply->errorString());
			return;
		}
		const auto data = reply->readAll();
		if (data.isEmpty()) {
			emit downloadFinished(id, false, QStringLiteral("empty body"));
			return;
		}
		QFile out(packDir(id) + QStringLiteral("model.bin"));
		if (!out.open(QIODevice::WriteOnly)) {
			emit downloadFinished(id, false, QStringLiteral("write failed"));
			return;
		}
		out.write(data);
		out.close();
		saveMarker(id);
		for (auto &p : _packs) {
			if (p.id == id) p.installed = true;
		}
		emit downloadFinished(id, true, QString());
		emit catalogUpdated();
	});
}
