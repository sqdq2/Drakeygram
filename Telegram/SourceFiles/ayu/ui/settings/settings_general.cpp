// This is the source code of AyuGram for Desktop / Drakeygram fork.
// Copyright retained from upstream contributors where applicable.

#include "ayu/ui/settings/settings_general.h"

#include "ayu/ayu_lang.h"
#include "ayu/ayu_settings.h"
#include "ayu/ui/settings/settings_ayu_section.h"
#include "lang/lang_keys.h"
#include "layout/layout_item.h"
#include "settings/settings_common.h"
#include "styles/style_settings.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "boxes/abstract_box.h"
#include "ui/layers/generic_box.h"
#include "core/application.h"
#include "mainwindow.h"
#include "platform/platform_translate_provider.h"

namespace Settings {
namespace {

void BuildTranslation(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::lng_translate_settings_subtitle());

	// Telegram = cloud. Offline = local packs + optional Bergamot binary.
	const auto items = std::vector{
		std::pair(TranslationProvider::Telegram, QString("Telegram")),
		std::pair(TranslationProvider::Google, QString("Offline (local Bergamot)")),
	};

	for (const auto &[provider, title] : items) {
		ayu.add(builder, {
			.title = rpl::single(title),
			.toggled = rpl::single(
				AyuSettings::getInstance().translationProvider() == provider),
			.callback = [=](bool enabled) {
				if (!enabled) {
					return;
				}
				auto &s = AyuSettings::getInstance();
				s.set_translationProvider(provider);
				s.save();
			},
		});
	}

	builder.add(object_ptr<Ui::FlatLabel>(
		builder.widget(),
		rpl::single(QString(
			"Offline: download language packs in-app (Mozilla models). "
			"Optional decode: run scripts/setup-bergamot.sh or set DRAKEY_BERGAMOT_BIN. "
			"See docs/OFFLINE_TRANSLATION.md")),
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
	ayu.add(builder, {
		.title = rpl::single(QString("Download language pack")),
		.label = rpl::single(QString("UI packs — offline MT packs are separate")),
		.callback = [=](bool) {
			Ui::Toast::Show(rpl::single(QString(
				"UI language packs: optional. For offline message translation "
				"download MT packs + optional Bergamot (docs/OFFLINE_TRANSLATION.md).")));
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
