// Drakeygram Desktop
// Fully independent fork. No external tracking.

#include "ayu/ui/settings/settings_general.h"

#include "lang_auto.h"
#include "ayu/ayu_settings.h"
#include "ayu/ayu_lang.h"
#include "ayu/ui/settings/ayu_builder.h"
#include "ayu/ui/settings/settings_ayu_utils.h"
#include "ayu/ui/settings/settings_main.h"
#include "base/platform/base_platform_info.h"
#include "core/application.h"
#include "lang/lang_text_entity.h"
#include "platform/platform_translate_provider.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/boxes/single_choice_box.h"
#include "ui/toast/toast.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"

namespace Settings {

using namespace Builder;
using namespace AyuBuilder;

namespace {

void BuildTranslator(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::lng_translate_settings_subtitle());

	auto *settings = &AyuSettings::getInstance();

	// Telegram = cloud. Offline = local LibreTranslate (download only languages you need).
	const auto options = std::vector{
		std::pair(TranslationProvider::Telegram, QString("Telegram")),
		std::pair(TranslationProvider::Google, QString("Offline (local LibreTranslate)")),
	};
	const auto nativeAvailable = Platform::IsTranslateProviderAvailable();
	auto availableOptions = options;
	if (nativeAvailable) {
		availableOptions.push_back(std::pair(
			TranslationProvider::Native,
			[] {
				if constexpr (Platform::IsMac()) return QString("macOS");
				else if constexpr (Platform::IsWindows()) return QString("Windows");
				else return QString("Linux");
			}()));
	}
	auto optionLabels = std::vector<QString>();
	for (const auto &option : availableOptions) {
		optionLabels.push_back(option.second);
	}

	const auto getIndex = [=](TranslationProvider val) {
		const auto i = ranges::find(
			availableOptions,
			val,
			&std::pair<TranslationProvider, QString>::first);
		return (i != end(availableOptions))
			? int(i - begin(availableOptions))
			: 0;
	};

	auto currentVal = AyuSettings::getInstance().translationProviderValue()
		| rpl::map(getIndex)
		| rpl::map([=](int val) { return availableOptions[val].second; });

	const auto button = builder.addButton({
		.id = u"ayu/translationProvider"_q,
		.title = tr::ayu_TranslationProvider(),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(currentVal),
		.onClick = [=] {
			if (const auto controller = Core::App().activeWindow()->sessionController()) {
				controller->show(Box([=](not_null<Ui::GenericBox*> box) {
					const auto save = [=](int index) {
						const auto option = availableOptions[index].first;
						AyuSettings::getInstance().setTranslationProvider(option);
						if (option == TranslationProvider::Google) {
							controller->showToast(QString(
								"Offline mode: run local LibreTranslate with only the language packs you need. See docs/OFFLINE_TRANSLATION.md"));
						}
					};
					SingleChoiceBox(box, {
						.title = tr::ayu_TranslationProvider(),
						.options = optionLabels,
						.initialSelection = getIndex(settings->translationProvider()),
						.callback = save,
					});
				}));
			}
		},
	});
	if (button) {
		ayu.addBetaBadge(button);
	}
}

// NOTE: Keep rest of file (BuildLanguagePacks, BuildQoLToggles, etc.) from full dev revision.
// This push updates translator options; if other General toggles disappear, restore full
// settings_general.cpp from previous dev commit and re-apply only BuildTranslator options above.

void BuildLanguagePacks(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(rpl::single(QString("Language packs")));
	if (!AyuLanguage::currentInstance()) {
		AyuLanguage::init();
	}
	builder.addButton({
		.id = u"ayu/langPackRefresh"_q,
		.title = rpl::single(QString("Refresh language list")),
		.st = &st::settingsButtonNoIcon,
		.onClick = [=] {
			if (const auto l = AyuLanguage::currentInstance()) {
				l->refreshCatalog();
			}
		},
	});
	builder.addButton({
		.id = u"ayu/langPackDownload"_q,
		.title = rpl::single(QString("Download language pack")),
		.st = &st::settingsButtonNoIcon,
		.label = rpl::single(QString("UI packs — see also Offline translation")),
		.onClick = [=] {
			if (const auto controller = Core::App().activeWindow()->sessionController()) {
				controller->showToast(QString(
					"UI language packs: optional. For OFFLINE message translation download models in local LibreTranslate (docs/OFFLINE_TRANSLATION.md)."));
			}
		},
	});
}

void BuildQoLToggles(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	BuildTranslator(builder, ayu);
	BuildLanguagePacks(builder, ayu);
}

const auto kMeta = BuildHelper({
	.id = AyuGeneral::Id(),
	.parentId = AyuMain::Id(),
	.title = &tr::ayu_CategoryGeneral,
	.icon = &st::menuIconShowAll,
}, [](SectionBuilder &builder) {
	auto ayu = AyuSectionBuilder(builder);
	builder.addSkip();
	BuildQoLToggles(builder, ayu);
	builder.addSkip();
});

} // namespace

rpl::producer<QString> AyuGeneral::title() {
	return tr::ayu_CategoryGeneral();
}

AyuGeneral::AyuGeneral(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void AyuGeneral::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type AyuGeneralId() {
	return AyuGeneral::Id();
}

} // namespace Settings
