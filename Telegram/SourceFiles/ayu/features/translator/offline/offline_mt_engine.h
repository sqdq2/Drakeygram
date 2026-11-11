// Drakeygram — Option 1 (lightweight): engine ships with the app package.
// No runtime engine download. No user scripts. Models still from Mozilla on demand.

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

	[[nodiscard]] OfflineDecodeResult decode(
		const QString &text,
		const QString &fromLang,
		const QString &toLang) const;

	[[nodiscard]] bool hasPack(const QString &fromLang, const QString &toLang) const;
	[[nodiscard]] QString packDir(const QString &fromLang, const QString &toLang) const;

	// Bundled next to the executable (preferred) or already present under tdata.
	[[nodiscard]] QString bundledEnginePath() const;
	[[nodiscard]] QString engineBinaryPath() const;
	[[nodiscard]] bool engineAvailable() const;

	// Idempotent: ensure tdata has a copy of the bundled engine (local file copy only).
	bool ensureBundledEngineReady() const;

private:
	OfflineMtEngine() = default;
	[[nodiscard]] QString findEngine() const;
};

} // namespace Ayu::Translator
