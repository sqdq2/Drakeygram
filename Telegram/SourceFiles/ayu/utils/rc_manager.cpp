// Drakeygram — remote config / tracker system permanently local-only.
// No network. No third-party endpoints.

#include "ayu/utils/rc_manager.h"

std::unordered_set<ID> default_developers = {};
std::unordered_set<ID> default_channels = {};

void RCManager::start() {
	DEBUG_LOG(("RCManager: permanently local-only (all remote config removed)"));
	initialized = true;
}

void RCManager::makeRequest() {}
void RCManager::sendRequest() {}

bool RCManager::tryRetryWithExteraFallback() {
	return false;
}

void RCManager::gotResponse() {}

bool RCManager::handleResponse(const QByteArray &) {
	return false;
}

bool RCManager::applyResponse(const QByteArray &) {
	return false;
}

void RCManager::gotFailure(QNetworkReply::NetworkError) {}

void RCManager::clearSentRequest() {
	_manager = nullptr;
}

RCManager::~RCManager() {
	_manager = nullptr;
}
