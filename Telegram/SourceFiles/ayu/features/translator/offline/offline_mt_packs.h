// Drakeygram — offline MT packs
// NETWORK POLICY:
//   - No connections at startup / idle / translate-time.
//   - HTTPS GET only when user explicitly refreshes catalog or downloads a pack.
//   - Never uploads chat text or any user content.

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <vector>

struct OfflineMtPackInfo {
	QString id;
	QString name;
	QString src;
	QString trg;
	bool installed = false;
};

class OfflineMtPacks final : public QObject {
	Q_OBJECT

public:
	static OfflineMtPacks &instance();

	// Local disk only — no network.
	void loadLocalCatalog();
	[[nodiscard]] std::vector<OfflineMtPackInfo> packs() const;
	[[nodiscard]] bool isInstalled(const QString &src, const QString &trg) const;
	[[nodiscard]] QString modelsRoot() const;
	[[nodiscard]] QString packDir(const QString &id) const;

	// NETWORK (HTTPS GET only) — call only from user actions (Settings buttons).
	void refreshCatalog();
	void downloadPack(const QString &id);

	static constexpr bool kNeverUploadUserText = true;
	static constexpr bool kNoBackgroundNetwork = true;

Q_SIGNALS:
	catalogUpdated();
	downloadProgress(const QString &id, qint64 received, qint64 total);
	downloadFinished(const QString &id, bool ok, const QString &error);

private:
	OfflineMtPacks(); // loads local catalog only — zero network
	void saveMarker(const QString &id);

	QNetworkAccessManager _nam;
	QNetworkReply *_catalogReply = nullptr;
	QNetworkReply *_downloadReply = nullptr;
	QString _downloadingId;
	std::vector<OfflineMtPackInfo> _packs;
};
