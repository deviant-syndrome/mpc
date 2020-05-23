#include <controls/disk/SaveControls.hpp>

#include <Util.hpp>
#include <disk/AbstractDisk.hpp>
#include <ui/NameGui.hpp>
#include <ui/disk/DiskGui.hpp>
#include <ui/sampler/SoundGui.hpp>
#include <sampler/Program.hpp>
#include <sampler/Sampler.hpp>
#include <sequencer/Sequence.hpp>
#include <sequencer/Track.hpp>
#include <sequencer/Sequencer.hpp>

using namespace mpc::controls::disk;
using namespace std;

SaveControls::SaveControls() 
	: AbstractDiskControls()
{
}

void SaveControls::function(int i)
{
	init();
	auto lDisk = disk.lock();
	switch (i) {
	case 0:
		ls.lock()->openScreen("load");
		break;
	case 2:
		ls.lock()->openScreen("format");
		break;
	case 3:
		ls.lock()->openScreen("setup");
		break;
	case 5:
		if (!lDisk)
			return;

		shared_ptr<mpc::sequencer::Sequence> seq;
		
		auto lProgram = program.lock();
		switch (diskGui->getType()) {
		case 0:
			nameGui->setName("ALL_SEQ_SONG1");
			ls.lock()->openScreen("saveallfile");
			break;
		case 1:
			seq = sequencer.lock()->getActiveSequence().lock();
			if (!seq->isUsed()) return;

			nameGui->setName(seq->getName());
			ls.lock()->openScreen("saveasequence");
			break;
		case 2:
			nameGui->setName("ALL_PGMS");
			ls.lock()->openScreen("saveapsfile");
			break;
		case 3:
			nameGui->setName(mpc::Util::getFileName(lProgram->getName()));
			ls.lock()->openScreen("saveaprogram");
			break;
		case 4:
			if (sampler.lock()->getSoundCount() == 0)
			{
				break;
			}
			nameGui->setName(sampler.lock()->getSoundName(sampler.lock()->getSoundIndex()));
			ls.lock()->openScreen("saveasound");
			break;
		}
	}
}

void SaveControls::turnWheel(int i)
{
	init();
	if (param.compare("type") == 0) {
		diskGui->setType(diskGui->getType() + i);
	}
	else if (param.compare("file") == 0) {

		int nr;
		
		
		switch (diskGui->getType()) {
		case 0:
		case 2:
			return;
		case 1:
			sequencer.lock()->setActiveSequenceIndex(sequencer.lock()->getActiveSequenceIndex() + i);
			break;
		case 3:
			nr = sequencer.lock()->getActiveSequence().lock()->getTrack(sequencer.lock()->getActiveTrackIndex()).lock()->getBusNumber();
			sampler.lock()->setDrumBusProgramNumber(nr, sampler.lock()->getDrumBusProgramNumber(nr) + i);
			break;
		case 4:
			sampler.lock()->setSoundIndex(sampler.lock()->getSoundIndex() + i);
			break;
		case 5:
			break;
		}
	}
}
