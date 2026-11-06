// Drakeygram Desktop
// Local language packs: catalog + download on demand, then offline use.

#pragma once

#include <QtNetwork/QNetworkReply>
#include <QtCore/QStringList>
#include <vector>

struct DrakeyLanguageInfo {
	QString id;
	QString name;
	QString nativeName;
	bool installed = false;
};

class AyuLanguage : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(AyuLanguage)

public:
	static AyuLanguage *currentInstance();
	static void init();
	static AyuLanguage *instance;

	// Download / refresh catalog (index.json), then individual packs.
	void refreshCatalog();
	void fetchLanguage(const QString &id, const QString &baseId = QString());
	void applyLanguageJson(QJsonDocument doc);

	[[nodiscard]] std::vector<DrakeyLanguageInfo> availableLanguages() const;
	[[nodiscard]] QStringList installedLanguageIds() const;
	[[nodiscard]] bool isInstalled(const QString &id) const;

	// Apply an already-downloaded pack as active extra strings.
	void useLocalLanguage(const QString &id);

Q_SIGNALS:
	catalogUpdated();
	languageDownloaded(const QString &id);
	languageDownloadFailed(const QString &id, const QString &error);

public Q_SLOTS:
	void fetchFinished();
	void fetchError(QNetworkReply::NetworkError e);
	void catalogFinished();
	void catalogError(QNetworkReply::NetworkError e);

private:
	AyuLanguage();
	~AyuLanguage() override = default;

	void loadCachedLanguage();
	void saveCachedLanguage(const QByteArray &json, const QString &langId);
	void loadCatalogFromDisk();
	void saveCatalogToDisk(const QByteArray &json);
	[[nodiscard]] QString getCacheDir() const;
	[[nodiscard]] QString getCachePath(const QString &langId) const;
	[[nodiscard]] QString getCatalogPath() const;

	QNetworkAccessManager networkManager;
	QNetworkReply *_chkReply = nullptr;
	QNetworkReply *_catalogReply = nullptr;
	QString _currentLangId;
	std::vector<DrakeyLanguageInfo> _catalog;
	enum class PendingKind { None, Pack, Catalog };
	PendingKind _pending = PendingKind::None;
};
