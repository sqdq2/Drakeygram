// Settings — Telegram + Offline only (no Google/Yandex names)

#include "ayu/ui/settings/settings_general.h"

#include "ayu/ayu_lang.h"
#include "ayu/ayu_settings.h"
#include "ayu/features/translator/offline/offline_mt_engine.h"
#include "ayu/features/translator/offline/offline_mt_packs.h"
#include "ayu/ui/settings/settings_ayu_section.h"
#include "lang/lang_keys.h"
#include "settings/settings_common.h"
#include "styles/style_settings.h"
#include "ui/widgets/labels.h"
#include "ui/toast/toast.h"
#include "platform/platform_translate_provider.h"

namespace Settings {
namespace {

void BuildTranslation(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::lng_translate_settings_subtitle());

	const auto items = std::vector{
		std::pair(TranslationProvider::Telegram, QString("Telegram")),
		std::pair(TranslationProvider::Offline, QString("Offline (local)")),
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
				if (provider == TranslationProvider::Offline) {
					Ayu::Translator::OfflineMtEngine::Instance().ensureBundledEngineReady();
				}
			},
		});
	}

	const auto engOk = Ayu::Translator::OfflineMtEngine::Instance().engineAvailable();
	ayu.add(builder, {
		.title = rpl::single(
			engOk ? QString("Offline engine: ready") : QString("Offline engine: not bundled")),
		.callback = [=](bool) {
			if (Ayu::Translator::OfflineMtEngine::Instance().ensureBundledEngineReady()) {
				Ui::Toast::Show(rpl::single(QString("Offline engine ready")));
			} else {
				Ui::Toast::Show(rpl::single(QString("Engine not shipped in this build")));
			}
		},
	});

	ayu.add(builder, {
		.title = rpl::single(QString("Refresh offline pack catalog")),
		.callback = [=](bool) {
			Ayu::Translator::OfflineMtPacks::instance().refreshCatalog();
		},
	});

	builder.add(object_ptr<Ui::FlatLabel>(
		builder.widget(),
		rpl::single(QString(
			"Providers: Telegram or Offline (local). "
			"Google/Yandex cloud translate names removed.")),
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
