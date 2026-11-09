// Drakeygram — Bergamot / Mozilla offline MT packs
// DOWNLOAD ONLY (HTTP GET). Never uploads chat text or any user content.

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <vector>

struct OfflineMtPackInfo {
	QString id;       // e.g. "en-ar"
	QString name;
	QString src;
	QString trg;
	bool installed = false;
};

class OfflineMtPacks final : public QObject {
	Q_OBJECT

public:
	static OfflineMtPacks &instance();

	void refreshCatalog();
	void downloadPack(const QString &id);
	[[nodiscard]] std::vector<OfflineMtPackInfo> packs() const;
	[[nodiscard]] bool isInstalled(const QString &src, const QString &trg) const;
	[[nodiscard]] QString modelsRoot() const;
	[[nodiscard]] QString packDir(const QString &id) const;

	// Privacy: no API uploads chat text. Only GET of model artifacts.
	static constexpr bool kNeverUploadUserText = true;

Q_SIGNALS:
	catalogUpdated();
	downloadProgress(const QString &id, qint64 received, qint64 total);
	downloadFinished(const QString &id, bool ok, const QString &error);

private:
	OfflineMtPacks();
	void loadLocalCatalog();
	void saveMarker(const QString &id);

	QNetworkAccessManager _nam;
	QNetworkReply *_catalogReply = nullptr;
	QNetworkReply *_downloadReply = nullptr;
	QString _downloadingId;
	std::vector<OfflineMtPackInfo> _packs;
};
