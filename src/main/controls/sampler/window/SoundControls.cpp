#include <controls/sampler/window/SoundControls.hpp>

#include <ui/NameGui.hpp>
#include <lcdgui/Field.hpp>
#include <ui/sampler/SoundGui.hpp>
#include <sampler/Sampler.hpp>

using namespace mpc::controls::sampler::window;
using namespace std;

SoundControls::SoundControls() 
	: AbstractSamplerControls()
{
}

void SoundControls::turnWheel(int i)
{
	init();
	auto lLs = ls.lock();
	if (param.compare("soundname") == 0) {
		nameGui->setName(lLs->lookupField("soundname").lock()->getText());
		nameGui->setParameterName("soundname");
		lLs->openScreen("name");
	}
}

void SoundControls::function(int i)
{
	super::function(i);
	
	auto lLs = ls.lock();
	string newSampleName;
	switch (i) {
	case 1:
		lLs->openScreen("deletesound");
		break;
	case 2:
		lLs->openScreen("convertsound");
		break;
	case 4:
		newSampleName = sampler.lock()->getSoundName(soundGui->getSoundIndex());
		//newSampleName = newSampleName->replaceAll("\\s+$", "");
		newSampleName = sampler.lock()->addOrIncreaseNumber(newSampleName);
		lLs->openScreen("copysound");
		soundGui->setNewName(newSampleName);
		break;
	}
}
