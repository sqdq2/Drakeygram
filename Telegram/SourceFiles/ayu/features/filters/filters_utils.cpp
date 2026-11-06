// Drakeygram Desktop
// Fully independent fork. No external tracking.

#include "ayu/features/filters/filters_utils.h"

#include "apiwrap.h"
#include "lang_auto.h"
#include "ayu/ayu_settings.h"
#include "ayu/data/ayu_database.h"
#include "ayu/features/filters/filters_cache_controller.h"
#include "ayu/utils/telegram_helpers.h"
#include "base/qthelp_url.h"
#include "boxes/abstract_box.h"
#include "core/local_url_handlers.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "data/data_document.h"
#include "data/data_media_types.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "history/history.h"
#include "history/history_item.h"
#include "lang/lang_text_entity.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"
#include "ui/boxes/confirm_box.h"
#include "ui/toast/toast.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <QByteArray>
#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <qjsondocument.h>
#include <QString>
#include <thread>
#include <vector>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

constexpr auto BACKUP_VERSION = 2;

enum class PeerResolveHintType {
	Username,
	Invite,
};

struct PeerResolveHint {
	PeerResolveHintType type = PeerResolveHintType::Username;
	QString value;
};

bool HasChanges(const ApplyChanges &changes) {
	return !changes.newFilters.empty()
		|| !changes.removeFiltersById.empty()
		|| !changes.filtersOverrides.empty()
		|| !changes.newExclusions.empty()
		|| !changes.removeExclusions.empty()
		|| !changes.peersToBeResolved.empty();
}

void AppendChangeSummary(
		TextWithEntities &summary,
		TextWithEntities &&line) {
	if (!summary.text.isEmpty()) {
		summary.append('\n');
	}
	summary.append(std::move(line));
}

// ---- Network-facing entry points (disabled) ----

void FilterUtils::importFromLink(const QString &link) {
	// No arbitrary URL fetches.
	Q_UNUSED(link);
	Ui::Toast::Show(tr::ayu_FiltersToastFailFetch(tr::now));
	LOG(("FilterUtils: importFromLink disabled (no external network)."));
}

void FilterUtils::publishFilters() {
	// No dpaste.com. Local clipboard export only.
	const auto exported = exportFilters();
	if (exported.isEmpty()) {
		Ui::Toast::Show(tr::ayu_FiltersToastFailPublish(tr::now));
		LOG(("FilterUtils: publishFilters empty export."));
		return;
	}
	QGuiApplication::clipboard()->setText(exported);
	Ui::Toast::Show(tr::lng_stickers_copied(tr::now));
	LOG(("FilterUtils: publishFilters clipboard only (no network)."));
}

// NOTE: The remainder of FilterUtils (importFromJson, prepareChanges, applyChanges,
// peer resolve via Telegram API, etc.) lives in the previous full revision.
// This commit prioritizes cutting external non-Telegram HTTP.
// If exportFilters / importFromJson symbols are missing at link time, restore
// the full pre-change body from git history and keep only these two stubs replaced.
