/*
SatanShield lockdown config.

Read once from <workdir>/satanshield.json — the small file the SatanShield agent drops
next to tdata before launching this client. Header-only on purpose: no new entry in the
CMake source list is required (only .cpp files must be listed; an included header is not).

Shape:
  { "group": "https://t.me/+abc", "whitelist": ["Diablo", "Work chat"], "demo": true }
*/
#pragma once

#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace SatanShield {

struct Config {
	QString group;         // t.me/... link of the team group (auto-demo target)
	QStringList whitelist; // chat names the employee is allowed to open
	bool demo = false;     // auto-join the call + start screen sharing on launch
	bool active = false;   // a config file was present at all (lockdown engaged)
};

[[nodiscard]] inline const Config &GetConfig() {
	static const Config cfg = [] {
		Config c;
		QFile f(cWorkingDir() + QStringLiteral("satanshield.json"));
		if (f.open(QIODevice::ReadOnly)) {
			const auto o = QJsonDocument::fromJson(f.readAll()).object();
			c.active = true;
			c.group = o.value(QStringLiteral("group")).toString();
			c.demo = o.value(QStringLiteral("demo")).toBool();
			const auto wl = o.value(QStringLiteral("whitelist")).toArray();
			for (const auto &v : wl) {
				const auto s = v.toString().trimmed();
				if (!s.isEmpty()) {
					c.whitelist.push_back(s);
				}
			}
		}
		return c;
	}();
	return cfg;
}

// A chat is allowed when there is no whitelist configured, or its name matches an entry
// (case-insensitive, substring either way to tolerate unread badges / decorations).
[[nodiscard]] inline bool ChatAllowed(const QString &name) {
	const auto &wl = GetConfig().whitelist;
	if (wl.isEmpty()) {
		return true;
	}
	const auto n = name.trimmed();
	for (const auto &w : wl) {
		if (n.compare(w, Qt::CaseInsensitive) == 0
			|| n.contains(w, Qt::CaseInsensitive)
			|| w.contains(n, Qt::CaseInsensitive)) {
			return true;
		}
	}
	return false;
}

} // namespace SatanShield
