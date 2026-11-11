// Offline neural decode — no network. Chat text never leaves the device.

#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryFile>

namespace Ayu::Translator {

OfflineMtEngine &OfflineMtEngine::Instance() {
	static OfflineMtEngine eng;
	return eng;
}

QString OfflineMtEngine::packDir(const QString &fromLang, const QString &toLang) const {
	return OfflineMtPacks::instance().packDir(fromLang + QLatin1Char('-') + toLang);
}

bool OfflineMtEngine::hasPack(const QString &fromLang, const QString &toLang) const {
	return OfflineMtPacks::instance().isInstalled(fromLang, toLang);
}

QString OfflineMtEngine::findBergamotBinary() const {
	const auto env = qEnvironmentVariable("DRAKEY_BERGAMOT_BIN").trimmed();
	if (!env.isEmpty() && QFile::exists(env)) {
		return env;
	}
	const auto names = { QStringLiteral("bergamot-translator"), QStringLiteral("bergamot") };
	for (const auto &name : names) {
		const auto p = QStandardPaths::findExecutable(name);
		if (!p.isEmpty()) return p;
	}
	// Optional local helper next to the app working dir
	const auto local = cWorkingDir() + QStringLiteral("DrakeygramBergamot/bergamot-translator");
	if (QFile::exists(local)) return local;
#ifdef Q_OS_WIN
	const auto localExe = cWorkingDir() + QStringLiteral("DrakeygramBergamot/bergamot-translator.exe");
	if (QFile::exists(localExe)) return localExe;
#endif
	return {};
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
	if (src.isEmpty()) src = QStringLiteral("en");
	const auto trg = toLang.trimmed();

	if (!hasPack(src, trg)) {
		r.error = QStringLiteral("pack not installed: %1-%2").arg(src, trg);
		return r;
	}

	const auto bin = findBergamotBinary();
	if (bin.isEmpty()) {
		r.error = QStringLiteral(
			"Bergamot binary not found. Install bergamot-translator or set DRAKEY_BERGAMOT_BIN. "
			"Models are on disk; decode needs the local engine.");
		LOG(("OfflineMT decode: no bergamot binary (pack present, still local-only)"));
		return r;
	}

	// Write input to a temp file — never send over network.
	QTemporaryFile inFile;
	inFile.setAutoRemove(true);
	if (!inFile.open()) {
		r.error = QStringLiteral("temp input failed");
		return r;
	}
	inFile.write(text.toUtf8());
	inFile.flush();

	const auto modelDir = packDir(src, trg);
	QProcess proc;
	proc.setProcessChannelMode(QProcess::SeparateChannels);
	// Generic CLI shape; users may wrap a small script as DRAKEY_BERGAMOT_BIN.
	QStringList args;
	args << QStringLiteral("--model-dir") << modelDir;
	args << QStringLiteral("--from") << src;
	args << QStringLiteral("--to") << trg;
	args << QStringLiteral("--input") << inFile.fileName();

	proc.start(bin, args);
	if (!proc.waitForStarted(5000)) {
		r.error = QStringLiteral("failed to start bergamot: %1").arg(proc.errorString());
		return r;
	}
	if (!proc.waitForFinished(120000)) {
		proc.kill();
		r.error = QStringLiteral("bergamot timeout");
		return r;
	}
	if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		r.error = QStringLiteral("bergamot exit %1: %2")
			.arg(proc.exitCode())
			.arg(QString::fromUtf8(proc.readAllStandardError()));
		return r;
	}
	const auto out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
	if (out.isEmpty()) {
		r.error = QStringLiteral("empty bergamot output");
		return r;
	}
	r.ok = true;
	r.text = out;
	return r;
}

} // namespace Ayu::Translator
