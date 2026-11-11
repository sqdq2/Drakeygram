// Option 1 lightweight: use engine shipped beside the app. Never download engine at runtime.
// Language packs remain official Mozilla GET (user-triggered). Chat text never leaves device.

#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"

#include "storage/localstorage.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryFile>

namespace Ayu::Translator {
namespace {

QString engineFileName() {
#if defined(Q_OS_WIN)
	return QStringLiteral("bergamot-translator.exe");
#else
	return QStringLiteral("bergamot-translator");
#endif
}

bool looksLikeEngine(const QString &path) {
	const QFileInfo fi(path);
	return fi.exists() && fi.isFile() && fi.size() > 1024;
}

} // namespace

OfflineMtEngine &OfflineMtEngine::Instance() {
	static OfflineMtEngine eng;
	return eng;
}

QString OfflineMtEngine::bundledEnginePath() const {
	// Lightweight layout: <appDir>/offline-engine/bergamot-translator[.exe]
	const auto appDir = QCoreApplication::applicationDirPath();
	const auto primary = appDir
		+ QStringLiteral("/offline-engine/")
		+ engineFileName();
	if (looksLikeEngine(primary)) {
		return primary;
	}
#ifdef Q_OS_MAC
	// App bundle Resources
	const auto res = appDir
		+ QStringLiteral("/../Resources/offline-engine/")
		+ engineFileName();
	if (looksLikeEngine(res)) {
		return QFileInfo(res).absoluteFilePath();
	}
#endif
	return primary; // expected install path even if missing
}

QString OfflineMtEngine::engineBinaryPath() const {
	return cWorkingDir()
		+ QStringLiteral("tdata/drakey/engine/")
		+ engineFileName();
}

QString OfflineMtEngine::findEngine() const {
	// 1) Already under tdata (copied from bundle once)
	if (looksLikeEngine(engineBinaryPath())) {
		return engineBinaryPath();
	}
	// 2) Shipped with the application package
	const auto bundled = bundledEnginePath();
	if (looksLikeEngine(bundled)) {
		return bundled;
	}
	return {};
}

bool OfflineMtEngine::engineAvailable() const {
	return !findEngine().isEmpty();
}

bool OfflineMtEngine::ensureBundledEngineReady() const {
	if (looksLikeEngine(engineBinaryPath())) {
		return true;
	}
	const auto bundled = bundledEnginePath();
	if (!looksLikeEngine(bundled)) {
		LOG(("OfflineMT: no bundled engine at %1").arg(bundled));
		return false;
	}
	// Local copy only — no network.
	QDir().mkpath(QFileInfo(engineBinaryPath()).absolutePath());
	QFile::remove(engineBinaryPath());
	if (!QFile::copy(bundled, engineBinaryPath())) {
		LOG(("OfflineMT: failed to copy bundled engine"));
		return false;
	}
#ifndef Q_OS_WIN
	QFile::setPermissions(
		engineBinaryPath(),
		QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
			| QFileDevice::ReadGroup | QFileDevice::ExeGroup
			| QFileDevice::ReadOther | QFileDevice::ExeOther);
#endif
	LOG(("OfflineMT: bundled engine ready at %1").arg(engineBinaryPath()));
	return true;
}

QString OfflineMtEngine::packDir(const QString &fromLang, const QString &toLang) const {
	return OfflineMtPacks::instance().packDir(fromLang + QLatin1Char('-') + toLang);
}

bool OfflineMtEngine::hasPack(const QString &fromLang, const QString &toLang) const {
	return OfflineMtPacks::instance().isInstalled(fromLang, toLang);
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

	// Prefer running the bundled/copied engine — never download.
	ensureBundledEngineReady();
	const auto bin = findEngine();
	if (bin.isEmpty()) {
		r.error = QStringLiteral(
			"Offline engine not shipped with this build. "
			"Rebuild with scripts/bundle-offline-engine.sh (maintainer).");
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
