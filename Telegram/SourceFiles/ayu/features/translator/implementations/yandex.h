// Drakeygram Desktop
// Fully independent fork. No external tracking.
// Yandex Translate backend is a local no-op (no network).

#pragma once

#include "ayu/features/translator/implementations/base.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QString>

namespace Ayu::Translator {

class YandexTranslator final : public MultiThreadTranslator
{
	Q_OBJECT

public:
	static YandexTranslator &instance();

	[[nodiscard]] QSet<QString> supportedLanguages() const override;

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit YandexTranslator(QObject *parent = nullptr);
};

}
