// Drakeygram Desktop
// LibreTranslate client (works with public API — no local server required).
// Class name kept for build/include compatibility with existing provider wiring.

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

	// Parallel requests for translating many messages / whole chat.
	[[nodiscard]] int getConcurrencyLimit() const override { return 6; }
	[[nodiscard]] int getMaxRetries() const override { return 2; }
	[[nodiscard]] int getBaseWaitTimeMs() const override { return 400; }

private:
	explicit GoogleTranslator(QObject *parent = nullptr);

	QNetworkAccessManager _nam;
};

}
