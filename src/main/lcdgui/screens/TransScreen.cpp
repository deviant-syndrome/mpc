#include "TransScreen.hpp"

#include "PunchScreen.hpp"

#include <sequencer/Track.hpp>

using namespace mpc::lcdgui::screens;

TransScreen::TransScreen(mpc::Mpc& mpc, const int layerIndex) 
	: ScreenComponent(mpc, "trans", layerIndex)
{
}

void TransScreen::open()
{
	findChild("function-keys")->Hide(sequencer->isPlaying());

	setBar0(0);
	setBar1(sequencer->getActiveSequence()->getLastBarIndex());

	displayTransposeAmount();
	displayTr();
}

void TransScreen::function(int i)
{
	init();
	auto punchScreen = mpc.screens->get<PunchScreen>("punch");

	switch (i)
	{
	case 0: // Intentional fall-through
	case 2:
		punchScreen->tab = i;
		openScreen(punchScreen->tabNames[i]);
		break;
	case 5:
		//if (amount == 0) break; // does 2kxl do that?
		openScreen("transpose-permanent");
		break;
	}
}

void TransScreen::turnWheel(int i)
{
	init();
	
	if (param.compare("tr") == 0)
	{
		setTr(tr + i);
	}
	else if (param.compare("transpose-amount") == 0)
	{
		setTransposeAmount(transposeAmount + i);
	}
	else if (param.compare("bar0") == 0)
	{
		auto candidate = bar0 + i;
		
		if (candidate < 0 || candidate > sequencer->getActiveSequence()->getLastBarIndex())
			return;

		setBar0(candidate);
	}
	else if (param.compare("bar1") == 0)
	{
		auto candidate = bar1 + i;
	
		if (candidate < 0 || candidate > sequencer->getActiveSequence()->getLastBarIndex())
			return;

		setBar1(candidate);
	}
}

void TransScreen::setTransposeAmount(int i)
{
	if (i < -12 || i > 12)
		return;
	
	transposeAmount = i;
	
	displayTransposeAmount();
}

void TransScreen::setTr(int i)
{
	if (i < -1 || i > 63)
		return;
	
	tr = i;
	displayTr();
}

void TransScreen::setBar0(int i)
{
	if (i < 0)
		return;
	
	bar0 = i;
	
	if (bar0 > bar1)
		bar1 = bar0;

	displayBars();
}

void TransScreen::setBar1(int i)
{
	if (i < 0)
		return;

	bar1 = i;
	
	if (bar1 < bar0)
		bar0 = bar1;
	
	displayBars();
}

void TransScreen::displayTransposeAmount()
{
	findField("transpose-amount")->setTextPadded(transposeAmount);
}

void TransScreen::displayTr()
{
	auto trName = std::string(tr == -1 ? "ALL" : sequencer->getActiveSequence()->getTrack(tr)->getName());
	findField("tr")->setTextPadded(tr + 1, "0");
	findLabel("track-name")->setText(trName);
}

void TransScreen::displayBars()
{
	findField("bar0")->setTextPadded(std::to_string(bar0 + 1), "0");
	findField("bar1")->setTextPadded(std::to_string(bar1 + 1), "0");
}

void TransScreen::play()
{
	ScreenComponent::play();
	findChild("function-keys")->Hide(sequencer->isPlaying());
}

void TransScreen::playStart()
{
	ScreenComponent::playStart();
	findChild("function-keys")->Hide(sequencer->isPlaying());
}

void TransScreen::rec()
{
	ScreenComponent::rec();
	findChild("function-keys")->Hide(sequencer->isPlaying());
}

void TransScreen::overDub()
{
	ScreenComponent::overDub();
	findChild("function-keys")->Hide(sequencer->isPlaying());
}

void TransScreen::stop()
{
	ScreenComponent::stop();
	findChild("function-keys")->Hide(sequencer->isPlaying());
}
