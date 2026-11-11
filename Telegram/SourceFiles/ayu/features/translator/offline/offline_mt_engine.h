// Drakeygram — in-app offline MT engine (no external setup required)
// Downloads engine binary into tdata once (HTTPS GET), then decodes locally.

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

namespace Ayu::Translator {

struct OfflineDecodeResult {
	bool ok = false;
	QString text;
	QString error;
};

class OfflineMtEngine final : public QObject {
	Q_OBJECT

public:
	static OfflineMtEngine &Instance();

	// Pure local decode after engine + pack are on disk.
	[[nodiscard]] OfflineDecodeResult decode(
		const QString &text,
		const QString &fromLang,
		const QString &toLang) const;

	[[nodiscard]] bool hasPack(const QString &fromLang, const QString &toLang) const;
	[[nodiscard]] QString packDir(const QString &fromLang, const QString &toLang) const;

	// Engine binary under tdata (auto-managed).
	[[nodiscard]] QString engineDir() const;
	[[nodiscard]] QString engineBinaryPath() const;
	[[nodiscard]] bool engineInstalled() const;

	// In-app install: HTTPS GET only, no user scripts/env.
	void ensureEngineInstalled();

Q_SIGNALS:
	engineInstallProgress(qint64 received, qint64 total);
	engineInstallFinished(bool ok, const QString &error);

private:
	OfflineMtEngine();
	[[nodiscard]] QString findLocalEngine() const;
	void startEngineDownload();

	class QNetworkAccessManager *_nam = nullptr;
	class QNetworkReply *_reply = nullptr;
	bool _installing = false;
};

} // namespace Ayu::Translator
