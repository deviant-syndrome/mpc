#include "MixerSetupScreen.hpp"

#include <lcdgui/screens/DrumScreen.hpp>

using namespace mpc::lcdgui::screens;
using namespace mpc::lcdgui;
using namespace moduru::lang;

MixerSetupScreen::MixerSetupScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "mixer-setup", layerIndex)
{
}

void MixerSetupScreen::open()
{
	displayMasterLevel();
	displayFxDrum();
	displayStereoMixSource();
	displayIndivFxSource();
	displayCopyPgmMixToDrum();
	displayRecordMixChanges();
	auto drumScreen = mpc.screens->get<DrumScreen>("drum");
	ls->setFunctionKeysArrangement(drumScreen->drum);
}

void MixerSetupScreen::displayMasterLevel() {

	auto level = getMasterLevelString();

	if (getMasterLevel() != -13)
	{
		level = StrUtil::padLeft(level, " ", 5);
	}

	findField("masterlevel")->setText(level);
}

void MixerSetupScreen::displayFxDrum() {
	findField("fxdrum")->setText(std::to_string(getFxDrum() + 1));
}

void MixerSetupScreen::displayStereoMixSource() {
	findField("stereomixsource")->setText(isStereoMixSourceDrum() ? "DRUM" : "PROGRAM");
}

void MixerSetupScreen::displayIndivFxSource() {
	findField("indivfxsource")->setText(isIndivFxSourceDrum() ? "DRUM" : "PROGRAM");
}

void MixerSetupScreen::displayCopyPgmMixToDrum() {
	findField("copypgmmixtodrum")->setText(isCopyPgmMixToDrumEnabled() ? "YES" : "NO");
}

void MixerSetupScreen::displayRecordMixChanges() {
	findField("recordmixchanges")->setText(isRecordMixChangesEnabled() ? "YES" : "NO");
}

void MixerSetupScreen::turnWheel(int i)
{
	init();

	if (param.compare("stereomixsource") == 0)
	{
		setStereoMixSourceDrum(i > 0);
	}
	else if (param.compare("indivfxsource") == 0)
	{
		setIndivFxSourceDrum(i > 0);
	}
	else if (param.compare("copypgmmixtodrum") == 0)
	{
		setCopyPgmMixToDrumEnabled(i > 0);
	}
	else if (param.compare("recordmixchanges") == 0)
	{
		setRecordMixChangesEnabled(i > 0);
	}
	else if (param.compare("masterlevel") == 0)
	{
		setMasterLevel(masterLevel + i);
	}
	else if (param.compare("fxdrum") == 0)
	{
		setFxDrum(fxDrum + i);
	}
}

void MixerSetupScreen::function(int i)
{
	init();

	if (i < 4)
	{
		auto drumScreen = mpc.screens->get<DrumScreen>("drum");
		drumScreen->drum = i;
		//mpc.setPreviousSamplerScreenName(currentScreenName);
		openScreen("mixer");
	}
}

int MixerSetupScreen::getMasterLevel()
{
	return masterLevel;
}

void MixerSetupScreen::setMasterLevel(int i)
{
	if (i < -13 || i > 2) return;
	masterLevel = i;
	displayMasterLevel();
}

int MixerSetupScreen::getFxDrum()
{
	return fxDrum;
}

void MixerSetupScreen::setFxDrum(int i)
{
	if (i < 0 || i > 3) return;
	fxDrum = i;
	displayFxDrum();
}

bool MixerSetupScreen::isStereoMixSourceDrum()
{
	return stereoMixSourceDrum;
}

void MixerSetupScreen::setStereoMixSourceDrum(bool b)
{
	stereoMixSourceDrum = b;
	displayStereoMixSource();
}

bool MixerSetupScreen::isIndivFxSourceDrum()
{
	return indivFxSourceDrum;
}

void MixerSetupScreen::setIndivFxSourceDrum(bool b)
{
	indivFxSourceDrum = b;
	displayIndivFxSource();
}

bool MixerSetupScreen::isCopyPgmMixToDrumEnabled()
{
	return copyPgmMixToDrumEnabled;
}

void MixerSetupScreen::setCopyPgmMixToDrumEnabled(bool b)
{
	copyPgmMixToDrumEnabled = b;
	displayCopyPgmMixToDrum();
}

bool MixerSetupScreen::isRecordMixChangesEnabled()
{
	return recordMixChangesEnabled;
}

void MixerSetupScreen::setRecordMixChangesEnabled(bool b)
{
	recordMixChangesEnabled = b;
	displayRecordMixChanges();
}

std::string MixerSetupScreen::getMasterLevelString()
{
	return masterLevelNames[masterLevel + 13];
}
