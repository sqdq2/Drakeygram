// Drakeygram — local session anti-theft guard
// Goals:
//  - Sessions survive app updates (vault under tdata)
//  - Session files bound to this machine + local secret
//  - Copied tdata alone must not unlock on another device
// Limits (honest):
//  - Telegram servers can still revoke sessions remotely
//  - Full memory scrape of an unlocked running app cannot be made impossible
//  - Protection is local cryptographic binding + passcode, not server magic

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QString>

namespace Ayu::Security {

struct SessionGuardStatus {
	bool sealPresent = false;
	bool sealValid = false;
	bool deviceMatch = false;
	bool passcodeRecommended = true;
	QString message;
};

class SessionGuard final {
public:
	static SessionGuard &Instance();

	// Stable-ish machine binding material (hashed; not reversible identity).
	[[nodiscard]] QByteArray deviceFingerprint() const;

	// Paths
	[[nodiscard]] QString guardRoot() const;   // tdata/drakey/session_guard/
	[[nodiscard]] QString sealPath() const;
	[[nodiscard]] QString vaultPath() const;   // update-safe copy of critical markers

	// Create / refresh seal after successful local unlock (call when user enables protection).
	bool createOrRefreshSeal(const QByteArray &localSecret);

	// Verify seal on startup. If missing, creates nothing (opt-in). If present and mismatch → fail.
	[[nodiscard]] SessionGuardStatus verifySeal(const QByteArray &localSecret) const;

	// Copy protection markers to vault so updates / reinstall same machine can restore binding.
	bool writeUpdateVault();
	bool restoreFromUpdateVault();

	// Wipe seal (user disable / factory reset).
	void destroySeal();

	// Derive key: SHA-256(deviceFingerprint || localSecret || salt)
	[[nodiscard]] QByteArray deriveBindingKey(
		const QByteArray &localSecret,
		const QByteArray &salt) const;

private:
	SessionGuard() = default;
};

// Convenience: prefer enabling Telegram Desktop local passcode + SessionGuard together.
void LogSessionProtectionBanner();

} // namespace Ayu::Security
