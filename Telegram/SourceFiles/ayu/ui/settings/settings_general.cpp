// Drakeygram settings — Option 1 offline (bundled engine)

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
		std::pair(TranslationProvider::Google, QString("Offline (bundled engine)")),
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
					Ayu::Translator::OfflineMtEngine::Instance().ensureBundledEngineReady();
				}
			},
		});
	}

	const auto engOk = Ayu::Translator::OfflineMtEngine::Instance().engineAvailable();
	ayu.add(builder, {
		.title = rpl::single(
			engOk
				? QString("Offline engine: ready (bundled)")
				: QString("Offline engine: not in this build")),
		.label = rpl::single(QString("Engine ships with the app — no download")),
		.callback = [=](bool) {
			auto &e = Ayu::Translator::OfflineMtEngine::Instance();
			if (e.ensureBundledEngineReady()) {
				Ui::Toast::Show(rpl::single(QString("Offline engine ready")));
			} else {
				Ui::Toast::Show(rpl::single(QString(
					"This build has no bundled engine. Maintainer: scripts/bundle-offline-engine.sh")));
			}
		},
	});

	ayu.add(builder, {
		.title = rpl::single(QString("Refresh offline pack catalog")),
		.callback = [=](bool) {
			Ayu::Translator::OfflineMtPacks::instance().refreshCatalog();
			Ui::Toast::Show(rpl::single(QString("Refreshing pack list…")));
		},
	});

	builder.add(object_ptr<Ui::FlatLabel>(
		builder.widget(),
		rpl::single(QString(
			"Lightweight Option 1: translation engine is part of the app package. "
			"Only language packs download from official Mozilla sources when you ask. "
			"Chat text never leaves the device.")),
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
