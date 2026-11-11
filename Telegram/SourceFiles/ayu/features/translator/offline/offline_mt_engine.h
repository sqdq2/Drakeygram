// Drakeygram — offline neural decode (local only, no network)
// Uses installed packs under tdata/drakey/mt_models/.
// Decode backends (first match wins):
//  1) DRAKEY_BERGAMOT_BIN or "bergamot-translator" / "bergamot" on PATH (CLI)
//  2) Fail closed if no local engine (never uploads text)

#pragma once

#include <QtCore/QString>

namespace Ayu::Translator {

struct OfflineDecodeResult {
	bool ok = false;
	QString text;
	QString error;
};

class OfflineMtEngine final {
public:
	static OfflineMtEngine &Instance();

	// Pure local. Must not open network connections.
	[[nodiscard]] OfflineDecodeResult decode(
		const QString &text,
		const QString &fromLang,
		const QString &toLang) const;

	[[nodiscard]] bool hasPack(const QString &fromLang, const QString &toLang) const;
	[[nodiscard]] QString packDir(const QString &fromLang, const QString &toLang) const;
	[[nodiscard]] QString findBergamotBinary() const;
};

} // namespace Ayu::Translator
