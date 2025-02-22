#include "SongScreen.hpp"

#include <sequencer/Step.hpp>
#include <sequencer/Song.hpp>

#include <Util.hpp>

using namespace mpc::lcdgui::screens;
using namespace moduru::lang;

SongScreen::SongScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "song", layerIndex)
{
}

void SongScreen::open()
{
	findField("loop")->setAlignment(Alignment::Centered);
	for (int i = 0; i < 3; i++)
	{
		findField("step" + std::to_string(i))->setAlignment(Alignment::Centered);
		findField("reps" + std::to_string(i))->setAlignment(Alignment::Centered);
	}

	init();
	displaySongName();
	displayNow0();
	displayNow1();
	displayNow2();
	displayTempoSource();
	displayTempo();
	displayLoop();
	displaySteps();
	sequencer->addObserver(this);
}

void SongScreen::close()
{
	sequencer->deleteObserver(this);
}

void SongScreen::up()
{
	init();
	
	if (param == "step1" || param == "sequence1" || param == "reps1")
	{
		if (offset == -1 || sequencer->isPlaying())
			return;
	
		setOffset(offset - 1);
		sequencer->setActiveSequenceIndex(sequencer->getSongSequenceIndex());
		sequencer->setBar(0);
	}
	else
	{
		ScreenComponent::up();
	}
}

void SongScreen::left()
{
	init();
	if (param == "sequence1")
	{
		ls->setFocus("step1");
	}
	else if (param == "reps1")
	{
		ls->setFocus("sequence1");
	}
	else if (param == "step1")
	{
		ls->setFocus("loop");
	}
	else
	{
		ScreenComponent::left();
	}
}


void SongScreen::right()
{
	init();
	
	if (param == "sequence1")
	{
		ls->setFocus("reps1");
	}
	else if (param == "step1")
	{
		ls->setFocus("sequence1");
	}
	else if (param == "loop")
	{
		ls->setFocus("step1");
	}
	else
	{
		ScreenComponent::right();
	}
}

void SongScreen::openWindow()
{
	if (sequencer->isPlaying())
		return;

	init();

	auto song = sequencer->getSong(activeSongIndex);

	if (param.find("now") == std::string::npos && !song->isUsed())
	{
		song->setUsed(true);
		auto songName = StrUtil::trim(defaultSongName) + StrUtil::padLeft(std::to_string(activeSongIndex + 1), "0", 2);
		song->setName(songName);
	}

	if (param == "loop")
		openScreen("loop-song");
	else if (param == "song")
		openScreen("song-window");
	else if (param == "tempo" || param == "tempo-source")
		openScreen("ignore-tempo-change");
}

void SongScreen::down()
{
	init();

	if (param == "step1" || param == "sequence1" || param == "reps1")
	{	
		auto song = sequencer->getSong(activeSongIndex);
		
		if (offset == song->getStepCount() - 1 || sequencer->isPlaying())
			return;

		setOffset(offset + 1);
		sequencer->setActiveSequenceIndex(sequencer->getSongSequenceIndex());
		sequencer->setBar(0);
	}
	else
	{
		ScreenComponent::down();
	}
}

void SongScreen::turnWheel(int i)
{
	init();
	
	auto song = sequencer->getSong(activeSongIndex);
	
	if (param.find("sequence") != std::string::npos)
	{
        if (offset + 1 > song->getStepCount() - 1)
        {
            song->insertStep(offset + 1);

            if (!song->isUsed())
            {
                song->setUsed(true);
                auto songName = StrUtil::trim(defaultSongName) + StrUtil::padLeft(std::to_string(activeSongIndex + 1), "0", 2);
                song->setName(songName);
            }

            displaySongName();
            displaySteps();
            displayTempo();
            return;
        }

		auto step = song->getStep(offset + 1).lock();
		
		step->setSequence(step->getSequence() + i);		
		sequencer->setActiveSequenceIndex(step->getSequence());
		sequencer->setBar(0);
		displayNow0();
		displayNow1();
		displayNow2();
		displaySteps();
	}
	else if (param.find("reps") != std::string::npos)
	{
		if (offset + 1 > song->getStepCount() - 1)
			return;

		song->getStep(offset + 1).lock()->setRepeats(song->getStep(offset + 1).lock()->getRepeats() + i);
		displaySteps();
	}
	else if (param == "song")
	{
		setActiveSongIndex(activeSongIndex + i);
		setOffset(-1);
		init();

		if (song->isUsed() && song->getStepCount() != 0)
			sequencer->setActiveSequenceIndex(song->getStep(0).lock()->getSequence());
	}
	else if (param == "tempo" && !sequencer->isTempoSourceSequenceEnabled())
	{
		sequencer->setTempo(sequencer->getTempo() + (i * 0.1));
	}
	else if (param == "tempo-source")
	{
		sequencer->setTempoSourceSequence(i > 0);
		displayTempoSource();
		displayTempo();
	}
	else if (param == "loop")
	{
		setLoop(i > 0);
	}
	else if (param == "step1")
	{
		setOffset(offset + i);
	}
}

void SongScreen::function(int i)
{
	if (sequencer->isPlaying())
		return;

	init();
	auto song = sequencer->getSong(activeSongIndex);
	
	switch (i)
	{
	case 4:
		if (sequencer->isPlaying())
			return;

		song->deleteStep(offset + 1);
		displaySteps();
		displayNow0();
		displayNow1();
		displayNow2();
		displayTempo();
		break;
	case 5:
		if (param != "step1" && param != "sequence1")
			return;

		song->insertStep(offset + 1);
		
		auto candidate = offset + 1;

		if (candidate + 1 >= song->getStepCount())
			candidate -= 1;

		setOffset(candidate);
	
		if (!song->isUsed())
		{
			song->setUsed(true);
			auto songName = StrUtil::trim(defaultSongName) + StrUtil::padLeft(std::to_string(activeSongIndex + 1), "0", 2);
			song->setName(songName);
		}

		displaySongName();
		displaySteps();
		displayTempo();
		break;
	}
}

void SongScreen::displayTempo()
{
	findField("tempo")->setText(Util::tempoString(sequencer->getTempo()));
}

void SongScreen::displayLoop()
{
	findField("loop")->setText(loop ? "YES" : "NO");
}

void SongScreen::displaySteps()
{
	auto song = sequencer->getSong(activeSongIndex);
	int steps = song->getStepCount();

	auto stepArray = std::vector<std::shared_ptr<mpc::lcdgui::Field>>{ findField("step0"), findField("step1"), findField("step2") };
	auto sequenceArray = std::vector<std::shared_ptr<mpc::lcdgui::Field>>{ findField("sequence0"), findField("sequence1"), findField("sequence2") };
	auto repsArray = std::vector<std::shared_ptr<mpc::lcdgui::Field>>{ findField("reps0"), findField("reps1"), findField("reps2") };

	for (int i = 0; i < 3; i++)
	{
		int stepIndex = i + offset;
	
		if (stepIndex >= 0 && stepIndex < steps)
		{
			stepArray[i]->setText(std::to_string(stepIndex + 1));
			auto seqname = sequencer->getSequence(song->getStep(stepIndex).lock()->getSequence())->getName();
			sequenceArray[i]->setText(StrUtil::padLeft(std::to_string(song->getStep(stepIndex).lock()->getSequence() + 1), "0", 2) + "-" + seqname);
			repsArray[i]->setText(std::to_string(song->getStep(stepIndex).lock()->getRepeats()));
		}
		else
		{
			stepArray[i]->setText("");
			sequenceArray[i]->setText(stepIndex == steps ? "   (end of song)" : "");
			repsArray[i]->setText("");
		}
	}
}

void SongScreen::displayTempoSource()
{
	findField("tempo-source")->setText(sequencer->isTempoSourceSequenceEnabled() ? "SEQ" : "MAS");
}

void SongScreen::displayNow0()
{
	int pastBars = 0;

	auto song = sequencer->getSong(activeSongIndex);

	for (int i = 0; i < offset + 1; i++)
	{
		if (i >= song->getStepCount())
			break;

		auto step = song->getStep(i).lock();
		auto seq = sequencer->getSequence(step->getSequence());

		if (!seq->isUsed())
			continue;

		auto bars = (seq->getLastBarIndex() + 1) * step->getRepeats();

		pastBars += bars;
	}

	pastBars += sequencer->getPlayedStepRepetitions() * (sequencer->getActiveSequence()->getLastBarIndex() + 1);
	
	findField("now0")->setTextPadded(sequencer->getCurrentBarIndex() + 1 + pastBars, "0");
}

void SongScreen::displayNow1()
{
	findField("now1")->setTextPadded(sequencer->getCurrentBeatIndex() + 1, "0");
}

void SongScreen::displayNow2()
{
	findField("now2")->setTextPadded(sequencer->getCurrentClockNumber(), "0");
}

void SongScreen::displaySongName()
{
	auto song = sequencer->getSong(activeSongIndex);
	findField("song")->setText(StrUtil::padLeft(std::to_string(activeSongIndex + 1), "0", 2) + "-" + song->getName());
}

void SongScreen::setOffset(int i)
{
	if (i < -1)
		i = -1;

	auto song = sequencer->getSong(activeSongIndex);

	if (i >= song->getStepCount() - 1)
		i = song->getStepCount() - 1;

	offset = i;

	displaySteps();
	displayTempo();
}

void SongScreen::setActiveSongIndex(int i)
{
	if (i < 0 || i > 19)
		return;
	
	activeSongIndex = i;

	displaySongName();
	displaySteps();
	displayTempoSource();
	displayTempo();
	displayNow0();
	displayNow1();
	displayNow2();
}

void SongScreen::setDefaultSongName(std::string s)
{
	defaultSongName = s;
}

void SongScreen::setLoop(bool b)
{
	loop = b;
	displayLoop();
}

void SongScreen::update(moduru::observer::Observable* observable, nonstd::any message)
{
	auto msg = nonstd::any_cast<std::string>(message);

	if (msg == "bar")
	{
		displayNow0();
	}
	else if (msg == "beat")
	{
		displayNow1();
	}
	else if (msg == "clock")
	{
		displayNow2();
	}
	else if (msg == "tempo")
	{
		displayTempo();
	}
	else if (msg == "play")
    {
        findField("sequence1")->setBlinking(true);
        findField("reps1")->setBlinking(true);
    }
	else if (msg == "stop")
    {
        findField("sequence1")->setBlinking(false);
        findField("reps1")->setBlinking(false);
    }
}

std::string SongScreen::getDefaultSongName()
{
	return defaultSongName;
}

int SongScreen::getOffset()
{
	return offset;
}

int SongScreen::getActiveSongIndex()
{
	return activeSongIndex;
}

bool SongScreen::isLoopEnabled()
{
	return loop;
}
