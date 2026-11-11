// In-app offline engine: install + decode without scripts or env vars.
// Network: HTTPS GET of engine binary only (user-triggered / first offline use).
// Chat text never uploaded.

#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryFile>
#include <QtCore/QSysInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ayu::Translator {
namespace {

// Official-ish release asset base (override with DRAKEY_ENGINE_URL if needed).
// Default: user can also drop a binary manually into engineDir(); app prefers tdata copy.
QString defaultEngineUrl() {
	const auto env = qEnvironmentVariable("DRAKEY_ENGINE_URL").trimmed();
	if (!env.isEmpty()) {
		return env;
	}
	// Placeholder catalog entry — real builds should point at your published engine asset.
	// Layout: one executable per OS. Until a binary is published, install reports a clear error
	// and Telegram translate remains available.
#if defined(Q_OS_WIN)
	return QStringLiteral(
		"https://github.com/sqdq2/Drakeygram/releases/download/engine-v1/bergamot-translator-windows-x64.exe");
#elif defined(Q_OS_MAC)
	return QStringLiteral(
		"https://github.com/sqdq2/Drakeygram/releases/download/engine-v1/bergamot-translator-macos");
#else
	return QStringLiteral(
		"https://github.com/sqdq2/Drakeygram/releases/download/engine-v1/bergamot-translator-linux-x64");
#endif
}

QString engineFileName() {
#if defined(Q_OS_WIN)
	return QStringLiteral("bergamot-translator.exe");
#else
	return QStringLiteral("bergamot-translator");
#endif
}

} // namespace

OfflineMtEngine &OfflineMtEngine::Instance() {
	static OfflineMtEngine eng;
	return eng;
}

OfflineMtEngine::OfflineMtEngine()
: _nam(new QNetworkAccessManager(this)) {
}

QString OfflineMtEngine::engineDir() const {
	return cWorkingDir() + QStringLiteral("tdata/drakey/engine/");
}

QString OfflineMtEngine::engineBinaryPath() const {
	return engineDir() + engineFileName();
}

bool OfflineMtEngine::engineInstalled() const {
	return QFileInfo::exists(engineBinaryPath())
		&& QFileInfo(engineBinaryPath()).isFile()
		&& QFileInfo(engineBinaryPath()).size() > 1024;
}

QString OfflineMtEngine::findLocalEngine() const {
	if (engineInstalled()) {
		return engineBinaryPath();
	}
	// Still accept a pre-placed binary without env vars.
	const auto fallback = cWorkingDir()
		+ QStringLiteral("DrakeygramBergamot/")
		+ engineFileName();
	if (QFileInfo::exists(fallback)) {
		return fallback;
	}
	return {};
}

QString OfflineMtEngine::packDir(const QString &fromLang, const QString &toLang) const {
	return OfflineMtPacks::instance().packDir(fromLang + QLatin1Char('-') + toLang);
}

bool OfflineMtEngine::hasPack(const QString &fromLang, const QString &toLang) const {
	return OfflineMtPacks::instance().isInstalled(fromLang, toLang);
}

void OfflineMtEngine::ensureEngineInstalled() {
	if (engineInstalled()) {
		emit engineInstallFinished(true, QString());
		return;
	}
	if (_installing) {
		return;
	}
	startEngineDownload();
}

void OfflineMtEngine::startEngineDownload() {
	const auto url = QUrl(defaultEngineUrl());
	if (url.scheme() != QStringLiteral("https")) {
		emit engineInstallFinished(false, QStringLiteral("Engine URL must be HTTPS"));
		return;
	}
	_installing = true;
	QDir().mkpath(engineDir());

	QNetworkRequest req{url};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Drakeygram-Engine/1.0"));
	req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

	LOG(("OfflineMT: in-app engine GET %1").arg(url.toString()));
	_reply = _nam->get(req);

	connect(_reply, &QNetworkReply::downloadProgress, this, [this](qint64 a, qint64 b) {
		emit engineInstallProgress(a, b);
	});

	connect(_reply, &QNetworkReply::finished, this, [this] {
		_installing = false;
		if (!_reply) {
			return;
		}
		const auto reply = _reply;
		_reply = nullptr;
		reply->deleteLater();

		if (reply->error() != QNetworkReply::NoError) {
			const auto err = reply->errorString();
			LOG(("OfflineMT: engine download failed: %1").arg(err));
			emit engineInstallFinished(
				false,
				QStringLiteral(
					"Could not download offline engine yet. "
					"Publish engine-v1 assets on GitHub Releases, or place bergamot-translator in tdata/drakey/engine/."));
			return;
		}
		const auto data = reply->readAll();
		if (data.size() < 1024) {
			emit engineInstallFinished(false, QStringLiteral("Engine download too small"));
			return;
		}
		const auto path = engineBinaryPath();
		QFile f(path);
		if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			emit engineInstallFinished(false, QStringLiteral("Cannot write engine"));
			return;
		}
		f.write(data);
		f.close();
#ifndef Q_OS_WIN
		QFile::setPermissions(
			path,
			QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
				| QFileDevice::ReadGroup | QFileDevice::ExeGroup
				| QFileDevice::ReadOther | QFileDevice::ExeOther);
#endif
		LOG(("OfflineMT: engine installed at %1 (%2 bytes)").arg(path).arg(data.size()));
		emit engineInstallFinished(true, QString());
	});
}

OfflineDecodeResult OfflineMtEngine::decode(
	const QString &text,
	const QString &fromLang,
	const QString &toLang) const {
	OfflineDecodeResult r;
	if (text.trimmed().isEmpty() || toLang.trimmed().isEmpty()) {
		r.error = QStringLiteral("empty input");
		return r;
	}
	auto src = fromLang.trimmed();
	if (src.isEmpty()) {
		src = QStringLiteral("en");
	}
	const auto trg = toLang.trimmed();

	if (!hasPack(src, trg)) {
		r.error = QStringLiteral("Pack not installed for %1→%2").arg(src, trg);
		return r;
	}

	auto bin = findLocalEngine();
	if (bin.isEmpty()) {
		r.error = QStringLiteral(
			"Offline engine not installed yet. Open Settings → Install offline engine "
			"(one-time in-app download).");
		return r;
	}

	QTemporaryFile inFile;
	inFile.setAutoRemove(true);
	if (!inFile.open()) {
		r.error = QStringLiteral("temp input failed");
		return r;
	}
	inFile.write(text.toUtf8());
	inFile.flush();

	QProcess proc;
	proc.setProcessChannelMode(QProcess::SeparateChannels);
	QStringList args;
	args << QStringLiteral("--model-dir") << packDir(src, trg);
	args << QStringLiteral("--from") << src;
	args << QStringLiteral("--to") << trg;
	args << QStringLiteral("--input") << inFile.fileName();

	proc.start(bin, args);
	if (!proc.waitForStarted(5000)) {
		r.error = QStringLiteral("failed to start engine: %1").arg(proc.errorString());
		return r;
	}
	if (!proc.waitForFinished(120000)) {
		proc.kill();
		r.error = QStringLiteral("engine timeout");
		return r;
	}
	if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		r.error = QStringLiteral("engine exit %1: %2")
			.arg(proc.exitCode())
			.arg(QString::fromUtf8(proc.readAllStandardError()).left(400));
		return r;
	}
	const auto out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
	if (out.isEmpty()) {
		r.error = QStringLiteral("empty engine output");
		return r;
	}
	r.ok = true;
	r.text = out;
	return r;
}

} // namespace Ayu::Translator
