// Drakeygram — device-bound session seal (anti-theft)
// Seal file format (binary-ish text):
//   DRAKEYSEAL1\n
//   salt=<hex>\n
//   mac=<hex>\n
// mac = HMAC-SHA256(bindingKey, deviceFingerprint || "drakeygram-session-seal-v1")
// Without correct machine fingerprint + local secret, mac verification fails.

#include "ayu/security/session_guard.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSysInfo>
#include <QRandomGenerator>
#include <QStorageInfo>

namespace Ayu::Security {
namespace {

QByteArray toHex(const QByteArray &b) {
	return b.toHex();
}

QByteArray fromHex(const QByteArray &h) {
	return QByteArray::fromHex(h);
}

// HMAC-SHA256 with secret key (HMAC construction).
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
	// Combine non-secret machine traits. Not a privacy ID published anywhere — local binding only.
	QStringList parts;
	parts << QSysInfo::machineHostName();
	parts << QSysInfo::machineUniqueId();
	parts << QSysInfo::currentCpuArchitecture();
	parts << QSysInfo::productType();
	parts << QSysInfo::kernelType();
	// Boot volume name helps reduce easy VM clone collisions (best-effort).
	const QStorageInfo root = QStorageInfo::root();
	parts << root.device();
	parts << root.fileSystemType();
	const auto raw = parts.join(QLatin1Char('|')).toUtf8();
	return QCryptographicHash::hash(raw, QCryptographicHash::Sha256);
}

QString SessionGuard::guardRoot() const {
	return cWorkingDir() + u"tdata/drakey/session_guard/"_q;
}

QString SessionGuard::sealPath() const {
	return guardRoot() + u"seal.v1"_q;
}

QString SessionGuard::vaultPath() const {
	return guardRoot() + u"update_vault/"_q;
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
		LOG(("SessionGuard: refuse empty local secret — set Desktop local passcode / secret"));
		return false;
	}
	QDir().mkpath(guardRoot());
	const auto salt = randomSalt(32);
	const auto key = deriveBindingKey(localSecret, salt);
	const auto msg = deviceFingerprint() + QByteArrayLiteral("drakeygram-session-seal-v1");
	const auto mac = hmacSha256(key, msg);

	QFile f(sealPath());
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		LOG(("SessionGuard: cannot write seal"));
		return false;
	}
	f.write("DRAKEYSEAL1\n");
	f.write("salt=" + toHex(salt) + "\n");
	f.write("mac=" + toHex(mac) + "\n");
	f.close();
	writeUpdateVault();
	LOG(("SessionGuard: seal created/refreshed (device-bound)"));
	return true;
}

SessionGuardStatus SessionGuard::verifySeal(const QByteArray &localSecret) const {
	SessionGuardStatus st;
	QFile f(sealPath());
	if (!f.exists()) {
		st.message = QStringLiteral("No seal yet — enable protection after setting a local secret/passcode.");
		return st;
	}
	st.sealPresent = true;
	if (!f.open(QIODevice::ReadOnly)) {
		st.message = QStringLiteral("Seal unreadable.");
		return st;
	}
	const auto lines = f.readAll().split('\n');
	f.close();
	QByteArray saltHex, macHex;
	for (const auto &line : lines) {
		if (line.startsWith("salt=")) saltHex = line.mid(5).trimmed();
		if (line.startsWith("mac=")) macHex = line.mid(4).trimmed();
	}
	if (saltHex.isEmpty() || macHex.isEmpty()) {
		st.message = QStringLiteral("Seal corrupted.");
		return st;
	}
	const auto salt = fromHex(saltHex);
	const auto expected = fromHex(macHex);
	const auto key = deriveBindingKey(localSecret, salt);
	const auto msg = deviceFingerprint() + QByteArrayLiteral("drakeygram-session-seal-v1");
	const auto actual = hmacSha256(key, msg);
	st.deviceMatch = true; // fingerprint used in derivation; mismatch → mac fail
	st.sealValid = (actual == expected && !localSecret.isEmpty());
	if (!st.sealValid) {
		st.deviceMatch = false;
		st.message = QStringLiteral(
			"SESSION PROTECTED: seal mismatch. Copied session data will not unlock on this device/secret.");
		LOG(("SessionGuard: VERIFY FAILED — possible theft or different machine/secret"));
	} else {
		st.message = QStringLiteral("Session seal OK (device-bound).");
	}
	return st;
}

bool SessionGuard::writeUpdateVault() {
	const auto v = vaultPath();
	QDir().mkpath(v);
	if (!QFile::exists(sealPath())) {
		return false;
	}
	const auto dest = v + u"seal.v1"_q;
	QFile::remove(dest);
	if (!QFile::copy(sealPath(), dest)) {
		LOG(("SessionGuard: vault copy failed"));
		return false;
	}
	// Marker so updaters know sessions must be preserved.
	QFile m(v + u"KEEP_TDATA"_q);
	if (m.open(QIODevice::WriteOnly)) {
		m.write("Do not delete tdata. Drakeygram sessions live here.\n");
		m.close();
	}
	return true;
}

bool SessionGuard::restoreFromUpdateVault() {
	const auto src = vaultPath() + u"seal.v1"_q;
	if (!QFile::exists(src)) {
		return false;
	}
	QDir().mkpath(guardRoot());
	QFile::remove(sealPath());
	if (!QFile::copy(src, sealPath())) {
		return false;
	}
	LOG(("SessionGuard: seal restored from update vault"));
	return true;
}

void SessionGuard::destroySeal() {
	QFile::remove(sealPath());
	QFile::remove(vaultPath() + u"seal.v1"_q);
	LOG(("SessionGuard: seal destroyed"));
}

void LogSessionProtectionBanner() {
	LOG(("SessionGuard: device-bound seal + local secret. "
		"Stolen tdata without secret+device binding must not verify. "
		"Always enable Telegram Desktop local passcode as base layer."));
}

} // namespace Ayu::Security
