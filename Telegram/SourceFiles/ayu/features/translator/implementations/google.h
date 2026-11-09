// Drakeygram Desktop
// Offline translator backend (local LibreTranslate only).
// Class name kept for existing provider wiring (TranslationProvider::Google → Offline).

#pragma once

#include "ayu/features/translator/implementations/base.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>

namespace Ayu::Translator {

class GoogleTranslator final : public MultiThreadTranslator
{
	Q_OBJECT

public:
	static GoogleTranslator &instance();

	[[nodiscard]] QSet<QString> supportedLanguages() const override { return {}; }

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

	[[nodiscard]] int getConcurrencyLimit() const override { return 4; }
	[[nodiscard]] int getMaxRetries() const override { return 1; }
	[[nodiscard]] int getBaseWaitTimeMs() const override { return 200; }

private:
	explicit GoogleTranslator(QObject *parent = nullptr);

	QNetworkAccessManager _nam;
};

}
