/*
SatanShield lockdown config.

Read once from <workdir>/satanshield.json — the small file the SatanShield agent drops
next to tdata before launching this client. Header-only on purpose: no new entry in the
CMake source list is required (only .cpp files must be listed; an included header is not).

Shape:
  { "group": "https://t.me/+abc", "whitelist": ["Diablo", "Work chat"], "demo": true }
*/
#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

namespace SatanShield {

// Append one line to <workdir>/satan.log so the agent can pull it and we can see
// exactly what the auto-join / auto-share core did on the employee's machine.
inline void Log(const QString &line) {
	QFile f(cWorkingDir() + QStringLiteral("satan.log"));
	if (f.open(QIODevice::Append | QIODevice::Text)) {
		QTextStream s(&f);
		s << QDateTime::currentDateTime().toString(Qt::ISODate)
			<< QStringLiteral(" | ") << line << '\n';
	}
}

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

// Write the live demo state to <workdir>/satan_status.json every tick. The agent reads
// it and reports Telegram/demonstration status to the dashboard. Includes a unix ts so
// the agent can tell a fresh file from a stale one (client killed).
inline void WriteStatus(bool running, bool inCall, bool sharing) {
	QFile f(cWorkingDir() + QStringLiteral("satan_status.json"));
	if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		const auto ts = QDateTime::currentDateTime().toSecsSinceEpoch();
		const auto json = QStringLiteral(
			"{\"running\":%1,\"inCall\":%2,\"sharing\":%3,\"ts\":%4}")
			.arg(running ? 1 : 0)
			.arg(inCall ? 1 : 0)
			.arg(sharing ? 1 : 0)
			.arg(ts);
		f.write(json.toUtf8());
	}
}

// Minimal JSON string escaping for the message log below.
[[nodiscard]] inline QString JsonEscape(const QString &s) {
	QString o;
	o.reserve(s.size() + 8);
	for (const QChar c : s) {
		switch (c.unicode()) {
		case '"': o += QStringLiteral("\\\""); break;
		case '\\': o += QStringLiteral("\\\\"); break;
		case '\n': o += QStringLiteral("\\n"); break;
		case '\r': o += QStringLiteral("\\r"); break;
		case '\t': o += QStringLiteral("\\t"); break;
		default:
			if (c.unicode() < 0x20) {
				o += QStringLiteral("\\u%1").arg(int(c.unicode()), 4, 16, QChar('0'));
			} else {
				o += c;
			}
		}
	}
	return o;
}

// Append one CLEAN message record (from the client's own model — correct chat,
// direction, sender and text) to <workdir>/satan_msgs.jsonl. The agent tails this
// and reports it, replacing the fragile UIA window-scraping.
inline void LogMessage(
		const QString &chat,
		bool out,
		const QString &sender,
		const QString &text,
		bool media) {
	QFile f(cWorkingDir() + QStringLiteral("satan_msgs.jsonl"));
	if (f.open(QIODevice::Append | QIODevice::Text)) {
		const auto ts = QDateTime::currentDateTime().toSecsSinceEpoch();
		const QString line = QStringLiteral(
			"{\"chat\":\"%1\",\"direction\":\"%2\",\"sender\":\"%3\","
			"\"text\":\"%4\",\"media\":%5,\"ts\":%6}\n")
			.arg(JsonEscape(chat))
			.arg(out ? QStringLiteral("out") : QStringLiteral("in"))
			.arg(JsonEscape(sender))
			.arg(JsonEscape(text))
			.arg(media ? QStringLiteral("true") : QStringLiteral("false"))
			.arg(ts);
		f.write(line.toUtf8());
	}
}

// Lockdown is engaged whenever the agent dropped a satanshield.json next to tdata.
// Used to block leaving the call, the in-app logout, profile edits, etc. A normal
// (non-monitored) user has no such file, so everything behaves stock.
[[nodiscard]] inline bool LockdownActive() {
	return GetConfig().active;
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
