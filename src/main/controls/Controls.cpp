#include "Controls.hpp"

#include <Mpc.hpp>

#include <controls/BaseControls.hpp>

#include <controls/GlobalReleaseControls.hpp>

#include <controls/other/InitControls.hpp>
#include <controls/other/VerControls.hpp>

#include <controls/midisync/SyncControls.hpp>

#include <controls/misc/PunchControls.hpp>

#include <controls/misc/SecondSeqControls.hpp>
#include <controls/misc/TransControls.hpp>
#include <controls/misc/window/TransposePermanentControls.hpp>

#include <controls/vmpc/DirectToDiskRecorderControls.hpp>
#include <controls/vmpc/RecordJamControls.hpp>
#include <controls/vmpc/RecordingFinishedControls.hpp>
#include <controls/vmpc/VmpcSettingsControls.hpp>

using namespace mpc::controls;

Controls::Controls()
{
	pressedPadVelos = std::vector<int>(16);

	controls["release"] = new GlobalReleaseControls();

	controls["init"] = new other::InitControls();
	controls["ver"] = new other::VerControls();

	controls["punch"] = new misc::PunchControls();
	controls["trans"] = new misc::TransControls();
	controls["2ndseq"] = new misc::SecondSeqControls();

	controls["transposepermanent"] = new misc::window::TransposePermanentControls();

	controls["sync"] = new midisync::SyncControls();

	controls["directtodiskrecorder"] = new vmpc::DirectToDiskRecorderControls();
	controls["recordjam"] = new vmpc::RecordJamControls();
	controls["recordingfinished"] = new vmpc::RecordingFinishedControls();
	controls["vmpc-settings"] = new vmpc::VmpcSettingsControls();
}

void Controls::setCtrlPressed(bool b) {
	ctrlPressed = b;
}

void Controls::setAltPressed(bool b) {
	altPressed = b;
}

bool Controls::isCtrlPressed() {
	return ctrlPressed;
}

bool Controls::isAltPressed() {
	return altPressed;
}

bool Controls::isErasePressed() {
	return erasePressed;
}

bool Controls::isRecPressed() {
	return recPressed;
}

bool Controls::isOverDubPressed() {
	return overDubPressed;
}

bool Controls::isTapPressed() {
	return tapPressed;
}

bool Controls::isGoToPressed() {
	return goToPressed;
}

bool Controls::isShiftPressed() {
	return shiftPressed;
}

bool Controls::isF3Pressed() {
	return f3Pressed;
}

bool Controls::isF4Pressed() {
	return f4Pressed;
}

bool Controls::isF5Pressed() {
	return f5Pressed;
}

bool Controls::isF6Pressed() {
	return f6Pressed;
}

std::set<int>* Controls::getPressedPads() {
	return &pressedPads;
}

std::vector<int>* Controls::getPressedPadVelos() {
	return &pressedPadVelos;
}

void Controls::setErasePressed(bool b) {
	erasePressed = b;
}

void Controls::setRecPressed(bool b) {
	recPressed = b;
}

void Controls::setOverDubPressed(bool b) {
	overDubPressed = b;
}

void Controls::setTapPressed(bool b) {
	tapPressed = b;
}

void Controls::setGoToPressed(bool b) {
	goToPressed = b;
}

void Controls::setShiftPressed(bool b) {
	shiftPressed = b;
}

void Controls::setF3Pressed(bool b) {
	f3Pressed = b;
}

void Controls::setF4Pressed(bool b) {
	f4Pressed = b;
}

void Controls::setF5Pressed(bool b) {
	f5Pressed = b;
}

void Controls::setF6Pressed(bool b) {
	f6Pressed = b;
}

BaseControls* Controls::getControls(std::string s)
{
	return controls[s];
}

GlobalReleaseControls* Controls::getReleaseControls()
{
	return (GlobalReleaseControls*) controls["release"];
}

Controls::~Controls()
{
	for (auto c : controls)
	{
		delete c.second;
	}
}
