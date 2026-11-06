// Drakeygram Desktop
// Fully independent fork. No external tracking.

#pragma once

#include <QtGui/QPixmap>

namespace Ayu::Ui::Itunes {

// Network disabled: always returns an empty pixmap.
QPixmap FetchCover(const QString &performer,
				   const QString &title,
				   int sizeHintPx = 300,
				   int timeoutMs = 5000);

}
