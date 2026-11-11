// Drakeygram — offline local translator only.
// Historical class name GoogleTranslator is an internal alias for Offline.
// There is NO Google API, NO network translate, NO upload of chat text.

#pragma once

#include "ayu/features/translator/implementations/base.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QString>

namespace Ayu::Translator {

class GoogleTranslator final : public MultiThreadTranslator {
	Q_OBJECT
public:
	static GoogleTranslator &instance();

	[[nodiscard]] QSet<QString> supportedLanguages() const override { return {}; }

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args) override;

	[[nodiscard]] int getConcurrencyLimit() const override { return 4; }
	[[nodiscard]] int getMaxRetries() const override { return 0; }
	[[nodiscard]] int getBaseWaitTimeMs() const override { return 0; }

private:
	explicit GoogleTranslator(QObject *parent = nullptr);
};

} // namespace Ayu::Translator
