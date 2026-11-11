// Drakeygram settings — translation + in-app offline engine

#include "ayu/ui/settings/settings_general.h"

#include "ayu/ayu_lang.h"
#include "ayu/ayu_settings.h"
#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"
#include "ayu/ui/settings/settings_ayu_section.h"
#include "lang/lang_keys.h"
#include "settings/settings_common.h"
#include "styles/style_settings.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/toast/toast.h"
#include "boxes/abstract_box.h"
#include "platform/platform_translate_provider.h"

namespace Settings {
namespace {

void BuildTranslation(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::lng_translate_settings_subtitle());

	const auto items = std::vector{
		std::pair(TranslationProvider::Telegram, QString("Telegram")),
		std::pair(TranslationProvider::Google, QString("Offline (in-app)")),
	};

	for (const auto &[provider, title] : items) {
		ayu.add(builder, {
			.title = rpl::single(title),
			.toggled = rpl::single(
				AyuSettings::getInstance().translationProvider() == provider),
			.callback = [=](bool enabled) {
				if (!enabled) return;
				auto &s = AyuSettings::getInstance();
				s.set_translationProvider(provider);
				s.save();
				if (provider == TranslationProvider::Google) {
					// Prepare engine in background — no terminal/scripts.
					Ayu::Translator::OfflineMtEngine::Instance().ensureEngineInstalled();
				}
			},
		});
	}

	ayu.add(builder, {
		.title = rpl::single(QString("Install offline engine")),
		.label = rpl::single(QString("One-time in-app download (HTTPS)")),
		.callback = [=](bool) {
			auto &eng = Ayu::Translator::OfflineMtEngine::Instance();
			if (eng.engineInstalled()) {
				Ui::Toast::Show(rpl::single(QString("Offline engine already installed")));
				return;
			}
			Ui::Toast::Show(rpl::single(QString("Downloading offline engine…")));
			QObject::connect(
				&eng,
				&Ayu::Translator::OfflineMtEngine::engineInstallFinished,
				builder.widget(),
				[=](bool ok, const QString &error) {
					if (ok) {
						Ui::Toast::Show(rpl::single(QString("Offline engine ready")));
					} else {
						Ui::Toast::Show(rpl::single(
							error.isEmpty()
								? QString("Engine install failed")
								: error));
					}
				},
				static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
			eng.ensureEngineInstalled();
		},
	});

	ayu.add(builder, {
		.title = rpl::single(QString("Refresh offline pack list")),
		.callback = [=](bool) {
			Ayu::Translator::OfflineMtPacks::instance().refreshCatalog();
			Ui::Toast::Show(rpl::single(QString("Refreshing pack catalog…")));
		},
	});

	builder.add(object_ptr<Ui::FlatLabel>(
		builder.widget(),
		rpl::single(QString(
			"Offline mode runs entirely in the app: download engine once, "
			download language packs as needed. No terminal setup. "
			"Chat text is never uploaded.")),
		st::boxDividerLabel));
}

void BuildLanguagePacks(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(rpl::single(QString("Language packs")));
	if (!AyuLanguage::currentInstance()) {
		AyuLanguage::init();
	}
	ayu.add(builder, {
		.title = rpl::single(QString("Refresh UI language list")),
		.callback = [=](bool) {
			if (const auto l = AyuLanguage::currentInstance()) {
				l->refreshCatalog();
			}
		},
	});
}

} // namespace

void SetupGeneral(not_null<Ui::VerticalLayout*> container) {
	SectionBuilder builder(container);
	AyuSectionBuilder ayu;
	BuildTranslation(builder, ayu);
	BuildLanguagePacks(builder, ayu);
}

} // namespace Settings
