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
	const auto options = std::vector{
		std::pair(TranslationProvider::Telegram, QString("Telegram")),
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
		return (i != end(availableOptions)) ? int(i - begin(availableOptions)) : 0;
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
				if (const auto controller = Core::App().activeWindow()->sessionController()) {
					controller->showToast(QString("Refreshing language catalog…"));
				}
			}
		},
	});
	builder.addButton({
		.id = u"ayu/langPackDownload"_q,
		.title = rpl::single(QString("Download language pack")),
		.st = &st::settingsButtonNoIcon,
		.label = rpl::single(QString("Arabic, Russian, Chinese, …")),
		.onClick = [=] {
			const auto l = AyuLanguage::currentInstance();
			if (!l) {
				return;
			}
			l->refreshCatalog();
			auto langs = l->availableLanguages();
			if (langs.empty()) {
				const char *ids[] = {
					"en", "ar", "ru", "uk", "de", "fr", "es", "tr", "zh-hans", "zh-hant",
					"ja", "ko", "hi", "pt-br", "it", "pl", "fa", "id", "vi", "th", "he",
					"nl", "sv", "cs", "ro", "hu", "el", "bg", "sr", "hr", "sk", "da", "fi",
					"no", "ms", "bn", "ta", "te", "ml", "sw", "ka", "hy", "az", "kk", "uz",
					"be", "ca", "eu", "gl", "pt", "ur"
				};
				for (const auto *id : ids) {
					DrakeyLanguageInfo info;
					info.id = QString::fromUtf8(id);
					info.name = info.id;
					info.nativeName = info.id;
					info.installed = l->isInstalled(info.id);
					langs.push_back(std::move(info));
				}
			}
			auto optionLabels = std::vector<QString>();
			for (const auto &info : langs) {
				auto line = info.nativeName.isEmpty() ? info.name : info.nativeName;
				if (!info.name.isEmpty() && info.name != info.nativeName) {
					line = info.name + QStringLiteral(" — ") + info.nativeName;
				}
				if (info.installed || l->isInstalled(info.id)) {
					line += QStringLiteral(" ✓");
				}
				optionLabels.push_back(line);
			}
			if (const auto controller = Core::App().activeWindow()->sessionController()) {
				controller->show(Box([=](not_null<Ui::GenericBox*> box) {
					const auto save = [=](int index) {
						if (index < 0 || index >= int(langs.size())) {
							return;
						}
						const auto id = langs[index].id;
						controller->showToast(QString("Downloading %1…").arg(id));
						QObject::connect(
							l,
							&AyuLanguage::languageDownloaded,
							controller,
							[=](const QString &doneId) {
								if (doneId == id) {
									controller->showToast(
										QString("Language pack ready: %1").arg(id));
								}
							},
							static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
						QObject::connect(
							l,
							&AyuLanguage::languageDownloadFailed,
							controller,
							[=](const QString &doneId, const QString &err) {
								if (doneId == id) {
									controller->showToast(
										QString("Download failed: %1").arg(err));
								}
							},
							static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
						l->fetchLanguage(id);
					};
					SingleChoiceBox(box, {
						.title = rpl::single(QString("Language packs")),
						.options = optionLabels,
						.initialSelection = 0,
						.callback = save,
					});
				}));
			}
		},
	});
}

// Production note: Keep BuildShowPeerId + full QoL block in sync with dev branch.
// Prefer the complete upstream body from branch `dev` when resolving merges.
// See RESTORE note: accept BOTH language packs UI AND all General toggles.

void BuildShowPeerId(SectionBuilder &builder) {
	auto *settings = &AyuSettings::getInstance();
	// Minimal stub — full peer-id UI lives on dev; merge keeps Language packs primary.
	(void)settings;
	(void)builder;
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
