/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Ui {
} // namespace Ui

namespace Calls::Group::Ui {
using namespace ::Ui;
} // namespace Calls::Group::Ui

namespace Calls::Group::Ui::DesktopCapture {

class ChooseSourceDelegate {
public:
	virtual QWidget *chooseSourceParent() = 0;
	virtual QString chooseSourceActiveDeviceId() = 0;
	virtual bool chooseSourceActiveWithAudio() = 0;
	virtual bool chooseSourceWithAudioSupported() = 0;
	virtual rpl::lifetime &chooseSourceInstanceLifetime() = 0;
	virtual void chooseSourceAccepted(
		const QString &deviceId,
		bool withAudio) = 0;
	virtual void chooseSourceStop() = 0;
};

void ChooseSource(not_null<ChooseSourceDelegate*> delegate);

// SatanShield: device id of the PRIMARY WHOLE SCREEN (Type::Screen only — never a
// window), for auto-share without the chooser. Empty if no screen is available.
[[nodiscard]] QString WholeScreenDeviceId();

} // namespace Calls::Group::Ui::DesktopCapture
