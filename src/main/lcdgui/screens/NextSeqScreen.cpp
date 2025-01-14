#include "NextSeqScreen.hpp"

#include <lcdgui/screens/window/TimingCorrectScreen.hpp>
#include <lcdgui/screens/SequencerScreen.hpp>

#include <sequencer/TempoChangeEvent.hpp>

#include <Util.hpp>

using namespace mpc::lcdgui;
using namespace mpc::lcdgui::screens;
using namespace mpc::lcdgui::screens::window;
using namespace moduru::lang;

NextSeqScreen::NextSeqScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "next-seq", layerIndex)
{
}

void NextSeqScreen::open()
{
	selectNextSqFromScratch = true;

	findLabel("tempo")->setSize(12, 9);
	findField("tempo")->setLocation(18, 11);
	findField("tempo")->setLeftMargin(1);

	displaySq();
	displayNow0();
	displayNow1();
	displayNow2();
	displayTempo();
	displayTempoSource();
	displayTiming();
	displayNextSq();
	
	sequencer->addObserver(this);

	if (sequencer->getNextSq() == -1)
		ls->setFocus("sq");
	else
		ls->setFocus("nextsq");
}

void NextSeqScreen::close()
{
	sequencer->deleteObserver(this);
}

void NextSeqScreen::turnWheel(int i)
{
	init();
	
	if (param == "sq")
	{
		if (sequencer->isPlaying())
		{
			sequencer->setNextSq(sequencer->getCurrentlyPlayingSequenceIndex() + i);
			ls->setFocus("nextsq");
		}
		else
			sequencer->setActiveSequenceIndex(sequencer->getActiveSequenceIndex() + i);
	}
	else if (param == "nextsq")
	{
		auto nextSq = sequencer->getNextSq();
		
		if (nextSq == -1 && i < 0)
			return;

		if (nextSq == -1 && selectNextSqFromScratch)
		{
			nextSq = sequencer->getActiveSequenceIndex();
			selectNextSqFromScratch = false;
		}
		else
			nextSq += i;

		sequencer->setNextSq(nextSq);

		displayNextSq();
	}
	else if (param == "timing")
	{
		auto screen = mpc.screens->get<TimingCorrectScreen>("timing-correct");
		auto noteValue = screen->getNoteValue();
		screen->setNoteValue(noteValue + i);
		setLastFocus("timing-correct", "notevalue");
		displayTiming();
	}
	else if (param == "tempo")
	{
		double oldTempo = sequencer->getTempo();
		double newTempo = oldTempo + (i * 0.1);
		sequencer->setTempo(newTempo);
		displayTempo();
	}
}

void NextSeqScreen::function(int i)
{
	init();

    if (i == 3 || i == 4)
    {
        auto nextSq = sequencer->getNextSq();
        sequencer->setNextSq(-1);
        selectNextSqFromScratch = true;
        displayNextSq();

        if (i == 3)
        {
            sequencer->stop();
            sequencer->move(0);
            sequencer->setActiveSequenceIndex(nextSq);
            sequencer->playFromStart();
        }
    }
	else if (i == 5)
	{
		openScreen("next-seq-pad");
	}
}

void NextSeqScreen::displaySq()
{
	std::string result = "";

	if (sequencer->isPlaying())
	{
		result.append(StrUtil::padLeft(std::to_string(sequencer->getCurrentlyPlayingSequenceIndex() + 1), "0", 2));
		result.append("-");
		result.append(sequencer->getCurrentlyPlayingSequence()->getName());
		findField("sq")->setText(result);
	}
	else
	{
		result.append(StrUtil::padLeft(std::to_string(sequencer->getActiveSequenceIndex() + 1), "0", 2));
		result.append("-");
		result.append(sequencer->getActiveSequence()->getName());
		findField("sq")->setText(result);
	}
}

void NextSeqScreen::displayNextSq()
{
	auto nextSq = sequencer->getNextSq();
    std::string res = "";

	if (nextSq != -1)
	{
		auto seqName = sequencer->getSequence(nextSq)->getName();
		res = StrUtil::padLeft(std::to_string(sequencer->getNextSq() + 1), "0", 2) + "-" + seqName;
	}
	
	findField("nextsq")->setText(res);
}

void NextSeqScreen::displayNow0()
{
	findField("now0")->setTextPadded(sequencer->getCurrentBarIndex() + 1, "0");
}

void NextSeqScreen::displayNow1()
{
	findField("now1")->setTextPadded(sequencer->getCurrentBeatIndex() + 1, "0");
}

void NextSeqScreen::displayNow2()
{
	findField("now2")->setTextPadded(sequencer->getCurrentClockNumber(), "0");
}

void NextSeqScreen::displayTempo()
{
	displayTempoLabel();
	findField("tempo")->setText(Util::tempoString(sequencer->getTempo()));
}

void NextSeqScreen::displayTempoLabel()
{
	auto currentRatio = -1;
	auto sequence = sequencer->isPlaying() ? sequencer->getCurrentlyPlayingSequence() : sequencer->getActiveSequence();
	for (auto& e : sequence->getTempoChangeEvents())
	{
		if (e->getTick() > sequencer->getTickPosition())
			break;

		currentRatio = e->getRatio();
	}

	if (currentRatio != 1000)
		findLabel("tempo")->setText(u8"c\u00C0:");
	else
		findLabel("tempo")->setText(u8" \u00C0:");
}

void NextSeqScreen::displayTempoSource()
{
	findField("tempo-source")->setText(sequencer->isTempoSourceSequenceEnabled() ? "(SEQ)" : "(MAS)");
}

void NextSeqScreen::displayTiming()
{
	auto timingCorrectScreen = mpc.screens->get<TimingCorrectScreen>("timing-correct");
	auto noteValue = timingCorrectScreen->getNoteValue();
	findField("timing")->setText(SequencerScreen::timingCorrectNames[noteValue]);
}

void NextSeqScreen::update(moduru::observer::Observable* o, nonstd::any arg)
{
    std::string s = nonstd::any_cast<std::string>(arg);

	if (s == "seqnumbername")
	{
		displaySq();
	}
	else if (s == "bar")
	{
		displayNow0();
	}
	else if (s == "beat")
	{
		displayNow1();
	}
	else if (s == "clock")
	{
		displayNow2();
	}
	else if (s == "now")
	{
		displayNow0();
		displayNow1();
		displayNow2();
	}
	else if (s == "nextsqvalue")
	{
		displayNextSq();
	}
	else if (s == "nextsq")
	{
		displayNextSq();
	}
	else if (s == "nextsqoff")
	{
		selectNextSqFromScratch = true;
		displayNextSq();
	}
	else if (s == "timing")
	{
		displayTiming();
	}
	else if (s == "tempo")
	{
		displayTempo();
	}
}
