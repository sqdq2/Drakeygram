// Drakeygram Desktop
// Fully independent fork. No external tracking.
// LibreTranslate client (class name kept for ABI/include compatibility).

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

private:
	explicit GoogleTranslator(QObject *parent = nullptr);

	QNetworkAccessManager _nam;
};

}
