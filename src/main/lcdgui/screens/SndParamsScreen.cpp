#include <lcdgui/screens/SndParamsScreen.hpp>

#include <Mpc.hpp>
#include <controls/Controls.hpp>
#include <ui/sampler/SoundGui.hpp>
#include <ui/sampler/window/EditSoundGui.hpp>
#include <sampler/Sampler.hpp>
#include <sampler/Sound.hpp>

using namespace mpc::lcdgui::screens;
using namespace std;

SndParamsScreen::SndParamsScreen(const int layerIndex) 
	: ScreenComponent("params", layerIndex)
{
}

void SndParamsScreen::openWindow()
{
	init();
	auto lLs = ls.lock();
	if (param.compare("snd") == 0) {
		setSoundIndex(soundGui->getSoundIndex(), sampler.lock()->getSoundCount());
		soundGui->setPreviousScreenName("params");
		lLs->openScreen("sound");
	}
}

void SndParamsScreen::function(int f)
{
	init();
	
	auto lLs = ls.lock();
	string newSampleName;
	vector<int> zone;
	switch (f) {
	case 0:
		lLs->openScreen("trim");
		break;
	case 1:
		lLs->openScreen("loop");
		break;
	case 2:
		lLs->openScreen("zone");
		break;
	case 3:
		sampler.lock()->sort();
		break;
	case 4:
		if (sampler.lock()->getSoundCount() == 0)
			return;

		newSampleName = sampler.lock()->getSoundName(soundGui->getSoundIndex());
		//newSampleName = newSampleName->replaceAll("\\s+$", "");
		newSampleName = sampler.lock()->addOrIncreaseNumber(newSampleName);
		editSoundGui->setNewName(newSampleName);
		editSoundGui->setPreviousScreenName("trim");
		lLs->openScreen("editsound");
		break;
	case 5:
		if (Mpc::instance().getControls().lock()->isF6Pressed()) {
			return;
		}
		
		Mpc::instance().getControls().lock()->setF6Pressed(true);
		
		zone = vector<int>{ soundGui->getZoneStart(soundGui->getZoneNumber()) , soundGui->getZoneEnd(soundGui->getZoneNumber()) };
		sampler.lock()->playX(soundGui->getPlayX(), &zone);
		break;
	}
}

void SndParamsScreen::turnWheel(int i)
{
	init();
	
	auto lSound = sound.lock();
	if (param.compare("playx") == 0) {
		soundGui->setPlayX(soundGui->getPlayX() + i);
	}
	else if (param.compare("snd") == 0 && i > 0) {
		sampler.lock()->setSoundGuiNextSound();
	}
	else if (param.compare("snd") == 0 && i < 0) {
		sampler.lock()->setSoundGuiPrevSound();
	}
	else if (param.compare("level") == 0) {
		lSound->setLevel(lSound->getSndLevel() + i);
	}
	else if (param.compare("tune") == 0) {
		lSound->setTune(lSound->getTune() + i);
	}
	else if (param.compare("beat") == 0) {
		lSound->setNumberOfBeats(lSound->getBeatCount() + i);
	}
}
