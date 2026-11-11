// Offline neural decode — no network. Chat text never leaves the device.
// Optional: requires a local Bergamot (or wrapper) binary.

#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryFile>

namespace Ayu::Translator {
namespace {

QStringList candidateBinaries() {
	QStringList out;
	const auto env = qEnvironmentVariable("DRAKEY_BERGAMOT_BIN").trimmed();
	if (!env.isEmpty()) {
		out << env;
	}
	// Official names + local helper locations (optional install).
	const auto names = {
		QStringLiteral("bergamot-translator"),
		QStringLiteral("bergamot"),
		QStringLiteral("translateLocally"),
	};
	for (const auto &name : names) {
		const auto p = QStandardPaths::findExecutable(name);
		if (!p.isEmpty()) {
			out << p;
		}
	}
	const auto wd = cWorkingDir();
	out << wd + QStringLiteral("DrakeygramBergamot/bergamot-translator");
#ifdef Q_OS_WIN
	out << wd + QStringLiteral("DrakeygramBergamot/bergamot-translator.exe");
#endif
	out << wd + QStringLiteral("../DrakeygramBergamot/bergamot-translator");
	out << wd + QStringLiteral("scripts/bergamot-wrapper.sh");
	return out;
}

} // namespace

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
	for (const auto &path : candidateBinaries()) {
		if (path.isEmpty()) {
			continue;
		}
		QFileInfo fi(path);
		if (fi.exists() && fi.isFile()) {
			return fi.absoluteFilePath();
		}
	}
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
	if (src.isEmpty()) {
		src = QStringLiteral("en");
	}
	const auto trg = toLang.trimmed();

	if (!hasPack(src, trg)) {
		r.error = QStringLiteral(
			"Language pack not installed for %1→%2. Download it in Settings (Offline packs).")
			.arg(src, trg);
		return r;
	}

	const auto bin = findBergamotBinary();
	if (bin.isEmpty()) {
		r.error = QStringLiteral(
			"Optional Bergamot engine not found. Run scripts/setup-bergamot.sh "
			or set DRAKEY_BERGAMOT_BIN. Packs are on disk; decode needs the local binary.");
		LOG(("OfflineMT: pack present, binary missing (optional feature)"));
		return r;
	}

	// Temp input file only — never network.
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
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	// Allow wrapper to find the real binary
	if (!env.contains(QStringLiteral("DRAKEY_BERGAMOT_REAL"))) {
		const auto realEnv = qEnvironmentVariable("DRAKEY_BERGAMOT_REAL").trimmed();
		if (!realEnv.isEmpty()) {
			env.insert(QStringLiteral("DRAKEY_BERGAMOT_REAL"), realEnv);
		}
	}
	proc.setProcessEnvironment(env);

	QStringList args;
	args << QStringLiteral("--model-dir") << modelDir;
	args << QStringLiteral("--from") << src;
	args << QStringLiteral("--to") << trg;
	args << QStringLiteral("--input") << inFile.fileName();

	LOG(("OfflineMT: local decode via %1").arg(bin));
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
