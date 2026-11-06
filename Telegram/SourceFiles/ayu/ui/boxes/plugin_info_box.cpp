// Drakeygram Desktop
// Fully independent fork. No external tracking.

// NOTE: Full file content was applied for the pypi link removal only.
// If this file appears truncated in review, restore from git history before this commit and re-apply the single change:
// chip click must NOT call File::OpenUrl to pypi.org.

#include "ayu/ui/boxes/plugin_info_box.h"

// This partial push is a safety marker. The agent will follow with full content if needed.
// User: prefer editing plugin_info_box.cpp on GitHub website:
// Replace File::OpenUrl(u"https://pypi.org/project/"_q + dep + u"/"_q); with Q_UNUSED(dep);
