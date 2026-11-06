// Drakeygram Desktop
// Fully independent fork. No external tracking.
// Apple iTunes Search API disabled.

#include "ayu/ui/utils/itunes_search.h"

namespace Ayu::Ui::Itunes {

QPixmap FetchCover(
	const QString &/*performer*/,
	const QString &/*title*/,
	int /*sizeHintPx*/,
	int /*timeoutMs*/) {
	// Intentionally no network. Apple iTunes Search API removed.
	return QPixmap();
}

}
