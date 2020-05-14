#include "AbstractSamplerControls.hpp"

#include <controls/Controls.hpp>

#include <Mpc.hpp>
#include <lcdgui/Field.hpp>
#include <ui/sampler/SamplerGui.hpp>
#include <ui/sampler/SoundGui.hpp>
#include <ui/sampler/window/EditSoundGui.hpp>
#include <ui/sampler/window/SamplerWindowGui.hpp>
#include <ui/sampler/window/ZoomGui.hpp>
#include <sampler/NoteParameters.hpp>
#include <sampler/Pad.hpp>
#include <sampler/Program.hpp>
#include <sampler/Sampler.hpp>
#include <sampler/Sound.hpp>
#include <sequencer/Sequence.hpp>
#include <sequencer/Track.hpp>
#include <sequencer/Sequencer.hpp>
#include <mpc/MpcSoundPlayerChannel.hpp>

#include <cmath>

using namespace mpc::controls::sampler;
using namespace std;

AbstractSamplerControls::AbstractSamplerControls() 
	: BaseControls()
{
	splitInc = vector<int>{ 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };
	auto uis = Mpc::instance().getUis().lock();
	zoomGui = uis->getZoomGui();
	soundGui = uis->getSoundGui();
	editSoundGui = uis->getEditSoundGui();
	swGui = uis->getSamplerWindowGui();
}

void AbstractSamplerControls::init()
{
	BaseControls::init();
	
	if (sampler.lock()->getSoundCount() != 0)
		sound = dynamic_pointer_cast<mpc::sampler::Sound>(sampler.lock()->getSound(soundGui->getSoundIndex()).lock());

	mpcSoundPlayerChannel = sampler.lock()->getDrum(samplerGui->getSelectedDrum());
	program = dynamic_pointer_cast<mpc::sampler::Program>(sampler.lock()->getProgram(mpcSoundPlayerChannel->getProgram()).lock());
	auto lProgram = program.lock();
	if (csn.compare("programassign") == 0) {
		lastPad = sampler.lock()->getLastPad(lProgram.get());
		auto note = lastPad->getNote();
		lastNp = dynamic_cast<mpc::sampler::NoteParameters*>(lProgram->getNoteParameters(note));
	}
	else {
		lastNp = sampler.lock()->getLastNp(lProgram.get());
	}
	splittable = param.compare("st") == 0 || param.compare("end") == 0 || param.compare("to") == 0 || param.compare("endlengthvalue") == 0;
}

int AbstractSamplerControls::getSoundIncrement(int notch_inc)
{
	auto soundInc = notch_inc;
	if (abs(notch_inc) != 1) {
		soundInc *= (int)(ceil(sound.lock()->getLastFrameIndex() / 15000.0));
	}
	return soundInc;
}

void AbstractSamplerControls::splitLeft()
{
	init();
	auto mtf = ls.lock()->lookupField(param).lock();
	auto controls = Mpc::instance().getControls().lock();
	if (controls->isShiftPressed()) {
		if (splittable) {
			if (!mtf->isSplit()) {
				mtf->setSplit(true);
			}
			else {
				mtf->setActiveSplit(mtf->getActiveSplit() - 1);
			}
		}
	}
	else {
		BaseControls::left();
	}
}

void AbstractSamplerControls::splitRight()
{
	init();
	auto mtf = ls.lock()->lookupField(param).lock();
	auto controls = Mpc::instance().getControls().lock();
	if (controls->isShiftPressed()) {
		if (splittable && mtf->isSplit()) {
			if (!mtf->setActiveSplit(mtf->getActiveSplit() + 1))
				mtf->setSplit(false);
		}
	}
	else {
		BaseControls::right();
	}
}
