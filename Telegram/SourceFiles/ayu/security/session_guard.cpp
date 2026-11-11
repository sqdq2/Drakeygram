// Drakeygram — device-bound session seal (anti-theft)
// Seal: DRAKEYSEAL1 + salt + HMAC-SHA256(deviceFingerprint + secret).
// Copied tdata without same machine + secret fails verification.

#include "ayu/security/session_guard.h"

#include "storage/localstorage.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRandomGenerator>
#include <QtCore/QStorageInfo>
#include <QtCore/QSysInfo>

namespace Ayu::Security {
namespace {

QByteArray toHex(const QByteArray &b) { return b.toHex(); }
QByteArray fromHex(const QByteArray &h) { return QByteArray::fromHex(h); }

QByteArray hmacSha256(const QByteArray &key, const QByteArray &message) {
	const int block = 64;
	QByteArray k = key;
	if (k.size() > block) {
		k = QCryptographicHash::hash(k, QCryptographicHash::Sha256);
	}
	if (k.size() < block) {
		k.append(QByteArray(block - k.size(), char(0)));
	}
	QByteArray o(block, char(0x5c));
	QByteArray i(block, char(0x36));
	for (int n = 0; n < block; ++n) {
		o[n] = char(uchar(o[n]) ^ uchar(k[n]));
		i[n] = char(uchar(i[n]) ^ uchar(k[n]));
	}
	const auto inner = QCryptographicHash::hash(i + message, QCryptographicHash::Sha256);
	return QCryptographicHash::hash(o + inner, QCryptographicHash::Sha256);
}

QByteArray randomSalt(int n = 32) {
	QByteArray s;
	s.resize(n);
	for (int i = 0; i < n; ++i) {
		s[i] = char(QRandomGenerator::global()->generate() & 0xFF);
	}
	return s;
}

} // namespace

SessionGuard &SessionGuard::Instance() {
	static SessionGuard g;
	return g;
}

QByteArray SessionGuard::deviceFingerprint() const {
	QStringList parts;
	parts << QSysInfo::machineHostName();
	parts << QSysInfo::machineUniqueId();
	parts << QSysInfo::currentCpuArchitecture();
	parts << QSysInfo::productType();
	parts << QSysInfo::kernelType();
	const QStorageInfo root = QStorageInfo::root();
	parts << root.device();
	parts << root.fileSystemType();
	return QCryptographicHash::hash(parts.join(QLatin1Char('|')).toUtf8(), QCryptographicHash::Sha256);
}

QString SessionGuard::guardRoot() const {
	return cWorkingDir() + QStringLiteral("tdata/drakey/session_guard/");
}

QString SessionGuard::sealPath() const {
	return guardRoot() + QStringLiteral("seal.v1");
}

QString SessionGuard::vaultPath() const {
	return guardRoot() + QStringLiteral("update_vault/");
}

QByteArray SessionGuard::deriveBindingKey(
	const QByteArray &localSecret,
	const QByteArray &salt) const {
	QCryptographicHash h(QCryptographicHash::Sha256);
	h.addData(QByteArrayLiteral("drakeygram-binding-v1"));
	h.addData(salt);
	h.addData(deviceFingerprint());
	h.addData(localSecret);
	return h.result();
}

bool SessionGuard::createOrRefreshSeal(const QByteArray &localSecret) {
	if (localSecret.isEmpty()) {
		LOG(("SessionGuard: empty secret rejected"));
		return false;
	}
	QDir().mkpath(guardRoot());
	const auto salt = randomSalt(32);
	const auto key = deriveBindingKey(localSecret, salt);
	const auto mac = hmacSha256(
		key,
		deviceFingerprint() + QByteArrayLiteral("drakeygram-session-seal-v1"));
	QFile f(sealPath());
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}
	f.write("DRAKEYSEAL1\n");
	f.write("salt=" + toHex(salt) + "\n");
	f.write("mac=" + toHex(mac) + "\n");
	f.close();
	writeUpdateVault();
	LOG(("SessionGuard: seal refreshed"));
	return true;
}

SessionGuardStatus SessionGuard::verifySeal(const QByteArray &localSecret) const {
	SessionGuardStatus st;
	QFile f(sealPath());
	if (!f.exists()) {
		st.message = QStringLiteral("No seal — enable after local passcode.");
		return st;
	}
	st.sealPresent = true;
	if (!f.open(QIODevice::ReadOnly)) {
		st.message = QStringLiteral("Seal unreadable");
		return st;
	}
	QByteArray saltHex, macHex;
	for (const auto &line : f.readAll().split('\n')) {
		if (line.startsWith("salt=")) saltHex = line.mid(5).trimmed();
		if (line.startsWith("mac=")) macHex = line.mid(4).trimmed();
	}
	f.close();
	if (saltHex.isEmpty() || macHex.isEmpty()) {
		st.message = QStringLiteral("Seal corrupted");
		return st;
	}
	const auto actual = hmacSha256(
		deriveBindingKey(localSecret, fromHex(saltHex)),
		deviceFingerprint() + QByteArrayLiteral("drakeygram-session-seal-v1"));
	st.sealValid = (actual == fromHex(macHex) && !localSecret.isEmpty());
	st.deviceMatch = st.sealValid;
	st.message = st.sealValid
		? QStringLiteral("Session seal OK")
		: QStringLiteral("PROTECTED: seal mismatch — session data not valid on this device/secret");
	if (!st.sealValid) {
		LOG(("SessionGuard: VERIFY FAILED"));
	}
	return st;
}

bool SessionGuard::writeUpdateVault() {
	QDir().mkpath(vaultPath());
	if (!QFile::exists(sealPath())) return false;
	const auto dest = vaultPath() + QStringLiteral("seal.v1");
	QFile::remove(dest);
	if (!QFile::copy(sealPath(), dest)) return false;
	QFile m(vaultPath() + QStringLiteral("KEEP_TDATA"));
	if (m.open(QIODevice::WriteOnly)) {
		m.write("Preserve tdata across updates.\n");
		m.close();
	}
	return true;
}

bool SessionGuard::restoreFromUpdateVault() {
	const auto src = vaultPath() + QStringLiteral("seal.v1");
	if (!QFile::exists(src)) return false;
	QDir().mkpath(guardRoot());
	QFile::remove(sealPath());
	return QFile::copy(src, sealPath());
}

void SessionGuard::destroySeal() {
	QFile::remove(sealPath());
	QFile::remove(vaultPath() + QStringLiteral("seal.v1"));
}

void LogSessionProtectionBanner() {
	LOG(("SessionGuard active policy: local passcode + device-bound seal; "
		"stolen tdata without binding must not verify"));
}

} // namespace Ayu::Security
