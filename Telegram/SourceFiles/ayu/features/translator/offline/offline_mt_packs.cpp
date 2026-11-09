// Drakeygram — offline MT pack manager (Bergamot / Mozilla models)
// STRICT: only HTTP GET for catalog + model files. Never POST/PUT user text.

#include "ayu/features/translator/offline/offline_mt_packs.h"

#include "base/const_string.h"
#include "core/application.h"
#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkRequest>

namespace {

// Official project catalog (lists pairs). Model bytes come from Mozilla (GET only).
QString catalogUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_MT_CATALOG_URL").trimmed();
	if (!env.isEmpty()) return env;
	return QStringLiteral(
		"https://raw.githubusercontent.com/sqdq2/Drakeygram/dev/lang/mt_index.json");
}

// Official Mozilla Firefox Translations models (download only).
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

} // namespace

OfflineMtPacks &OfflineMtPacks::instance() {
	static OfflineMtPacks inst;
	return inst;
}

OfflineMtPacks::OfflineMtPacks() {
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
	const auto marker = packDir(id) + QStringLiteral(".installed");
	return QFile::exists(marker);
}

std::vector<OfflineMtPackInfo> OfflineMtPacks::packs() const {
	return _packs;
}

void OfflineMtPacks::loadLocalCatalog() {
	_packs.clear();
	const auto local = modelsRoot() + QStringLiteral("catalog.json");
	QFile f(local);
	if (!f.open(QIODevice::ReadOnly)) {
		return;
	}
	QJsonParseError err{};
	const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
	f.close();
	if (err.error != QJsonParseError::NoError || !doc.isObject()) {
		return;
	}
	const auto arr = doc.object().value(QStringLiteral("languages")).toArray();
	for (const auto &v : arr) {
		const auto o = v.toObject();
		OfflineMtPackInfo info;
		info.id = o.value(QStringLiteral("id")).toString();
		info.name = o.value(QStringLiteral("name")).toString();
		info.src = o.value(QStringLiteral("src")).toString();
		info.trg = o.value(QStringLiteral("trg")).toString();
		if (info.id.isEmpty() && !info.src.isEmpty() && !info.trg.isEmpty()) {
			info.id = info.src + QStringLiteral("-") + info.trg;
		}
		if (info.id.isEmpty()) continue;
		info.installed = QFile::exists(packDir(info.id) + QStringLiteral(".installed"));
		_packs.push_back(std::move(info));
	}
}

void OfflineMtPacks::saveMarker(const QString &id) {
	QDir().mkpath(packDir(id));
	QFile f(packDir(id) + QStringLiteral(".installed"));
	if (f.open(QIODevice::WriteOnly)) {
		f.write("download-only\n"); // marker only — no user content
		f.close();
	}
}

void OfflineMtPacks::refreshCatalog() {
	// GET only — catalog JSON, no user payload.
	QNetworkRequest req{QUrl(catalogUrl())};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram-OfflineMT/1.0"));
	req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	if (_catalogReply) {
		_catalogReply->abort();
		_catalogReply->deleteLater();
	}
	_catalogReply = _nam.get(req); // GET
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
		LOG(("OfflineMT: catalog updated, %1 packs").arg(_packs.size()));
	});
}

void OfflineMtPacks::downloadPack(const QString &id) {
	if (id.isEmpty()) {
		emit downloadFinished(id, false, QStringLiteral("empty id"));
		return;
	}
	if (_downloadReply) {
		emit downloadFinished(id, false, QStringLiteral("another download in progress"));
		return;
	}

	// Resolve src/trg from catalog.
	QString src, trg, name;
	for (const auto &p : _packs) {
		if (p.id == id) {
			src = p.src;
			trg = p.trg;
			name = p.name;
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

	// Official Mozilla model path pattern (GET only).
	// Example layout under firefox-translations-models / models.
	const auto fileUrl = mozillaModelsBase()
		+ QLatin1Char('/')
		+ src + QStringLiteral("-" ) + trg
		+ QStringLiteral("/model.intgemm.alphas.bin");

	QDir().mkpath(packDir(id));
	_downloadingId = id;

	QNetworkRequest req{QUrl(fileUrl)};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram-OfflineMT/1.0"));
	req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

	LOG(("OfflineMT: GET model (no upload) %1").arg(fileUrl));
	_downloadReply = _nam.get(req); // GET only — never sends chat text

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
			// Still mark as "requested" so UI can show retry; some pairs use different filenames.
			LOG(("OfflineMT: model GET failed for %1: %2 (pair may use alternate file names on Mozilla host)")
				.arg(id, reply->errorString()));
			// Write marker only if we got some payload on success path.
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
			emit downloadFinished(id, false, QStringLiteral("cannot write"));
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
		LOG(("OfflineMT: pack installed %1 (%2 bytes) — download only, no user text uploaded")
			.arg(id).arg(data.size()));
	});
}
