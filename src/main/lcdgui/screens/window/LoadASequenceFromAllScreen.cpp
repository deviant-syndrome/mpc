#include "LoadASequenceFromAllScreen.hpp"

#include <sequencer/Sequence.hpp>

#include <lcdgui/screens/window/LoadASequenceScreen.hpp>

using namespace mpc::lcdgui::screens::window;
using namespace mpc::lcdgui::screens;

LoadASequenceFromAllScreen::LoadASequenceFromAllScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "load-a-sequence-from-all", layerIndex)
{
}

void LoadASequenceFromAllScreen::open()
{
	sourceSeqIndex = 0;
	displayFile();
	displayLoadInto();
}

void LoadASequenceFromAllScreen::turnWheel(int i)
{
	init();

	if (param.compare("file") == 0)
	{
		setSourceSeqIndex(sourceSeqIndex + i);
	}
	else if (param.compare("load-into") == 0)
	{
		auto loadASequenceScreen = mpc.screens->get<LoadASequenceScreen>("load-a-sequence");
		loadASequenceScreen->setLoadInto(loadASequenceScreen->loadInto + i);
		displayLoadInto();
	}
}

void LoadASequenceFromAllScreen::function(int i)
{
	init();
	
	switch (i)
	{
	case 3:
		openScreen("mpc2000xl-all-file");
		break;
	case 4:
		auto candidate = sequencesFromAllFile[sourceSeqIndex];

		if (candidate)
		{
			auto loadASequenceScreen = mpc.screens->get<LoadASequenceScreen>("load-a-sequence");
			sequencer->setSequence(loadASequenceScreen->loadInto, candidate);
			openScreen("load");
		}
		break;
	}
}

void LoadASequenceFromAllScreen::displayFile()
{
    if (sourceSeqIndex >= sequencesFromAllFile.size())
        return;
    
	findField("file")->setTextPadded(sourceSeqIndex + 1, "0");

    auto candidate = sequencesFromAllFile[sourceSeqIndex];
	
    auto name = candidate ? candidate->getName() : "(Unused)";
	
    findLabel("file0")->setText("-" + name);
}

void LoadASequenceFromAllScreen::displayLoadInto()
{
	auto loadASequenceScreen = mpc.screens->get<LoadASequenceScreen>("load-a-sequence");
	findField("load-into")->setTextPadded(loadASequenceScreen->loadInto + 1, "0");
	findLabel("load-into0")->setText("-" + sequencer->getSequence(loadASequenceScreen->loadInto)->getName());
}

void LoadASequenceFromAllScreen::setSourceSeqIndex(int i)
{
	if (i < 0 || i >= sequencesFromAllFile.size())
		return;

	sourceSeqIndex = i;
	displayFile();
}
