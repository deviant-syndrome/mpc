#include "EditVelocityScreen.hpp"

#include <sequencer/Track.hpp>
#include <sequencer/NoteEvent.hpp>
#include <sequencer/SeqUtil.hpp>

#include <Util.hpp>

#include <lang/StrUtil.hpp>

using namespace mpc::lcdgui::screens::window;
using namespace mpc::sequencer;

using namespace moduru::lang;

EditVelocityScreen::EditVelocityScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "edit-velocity", layerIndex)
{
}

void EditVelocityScreen::setNote0(int i)
{
    init();

    if (param == "note0")
    {
        WithTimesAndNotes::setNote0(i);
    }
}

void EditVelocityScreen::open()
{
	auto bus = sequencer->getActiveTrack()->getBus();

	if (bus == 0)
	{
		findField("note0")->setAlignment(Alignment::Centered, 18);
		findField("note1")->setAlignment(Alignment::Centered, 18);
		findField("note0")->setLocation(62, 42);
	}
	else
	{
		findField("note0")->setAlignment(Alignment::None);
		findField("note1")->setAlignment(Alignment::None);
		findField("note0")->setLocation(61, 42);
	}

	findField("note1")->setLocation(116, 42);

	auto seq = sequencer->getActiveSequence();

	setTime0(0);
	setTime1(seq->getLastTick());
	
	displayEditType();
	displayValue();
	displayTime();
	displayNotes();
}

void EditVelocityScreen::function(int i)
{
	ScreenComponent::function(i);
	
	switch (i)
	{
	case 4:
		for (auto& event : track->getEvents())
		{
			auto ne = std::dynamic_pointer_cast<NoteEvent>(event);

			if (ne)
			{
				if (ne->getTick() >= time0 && ne->getTick() <= time1)
				{
					if (editType == 0)
					{
						ne->setVelocity(ne->getVelocity() + value);
					} 
					else if (editType == 1)
					{
						ne->setVelocity(ne->getVelocity() - value);
					}
					else if (editType == 2)
					{
						ne->setVelocity(ne->getVelocity() * (value / 100.0));
					}
					else if (editType == 3)
					{
						ne->setVelocity(value);
					}
				}
			}
		}

		openScreen("sequencer");
		break;
	}
}

void EditVelocityScreen::turnWheel(int i)
{
	init();

	if (param == "edittype")
		setEditType(editType + i);
	else if (param == "value")
		setValue(value + i);
	
	checkAllTimesAndNotes(mpc, i);
}

void EditVelocityScreen::displayTime()
{
	auto sequence = sequencer->getActiveSequence().get();
	findField("time0")->setTextPadded(SeqUtil::getBarFromTick(sequence, time0) + 1, "0");
	findField("time1")->setTextPadded(SeqUtil::getBeat(sequence, time0) + 1, "0");
	findField("time2")->setTextPadded(SeqUtil::getClock(sequence, time0), "0");
	findField("time3")->setTextPadded(SeqUtil::getBarFromTick(sequence, time1) + 1, "0");
	findField("time4")->setTextPadded(SeqUtil::getBeat(sequence, time1) + 1, "0");
	findField("time5")->setTextPadded(SeqUtil::getClock(sequence, time1), "0");
}

void EditVelocityScreen::displayNotes()
{
	init();
	
	if (track->getBus() == 0)
	{
		findField("note0")->setSize(47, 9);
		findLabel("note1")->Hide(false);
		findField("note1")->Hide(false);
		findField("note0")->setText(moduru::lang::StrUtil::padLeft(std::to_string(note0), " ", 3) + "(" + mpc::Util::noteNames()[note0] + u8"\u00D4");
		findField("note1")->setText(moduru::lang::StrUtil::padLeft(std::to_string(note1), " ", 3) + "(" + mpc::Util::noteNames()[note1] + u8"\u00D4");
	}
	else
	{
		findField("note0")->setSize(37, 9);
		
		if (note0 == 34)
		{
            findField("note0")->setText("ALL");
		}
		else
		{
            auto padName = sampler->getPadName(program->getPadIndexFromNote(note0));
            findField("note0")->setText(std::to_string(note0) + "/" + padName);
		}
		
		findLabel("note1")->Hide(true);
		findField("note1")->Hide(true);
	}
}

void EditVelocityScreen::setEditType(int i)
{
	if (i < 0 || i > 3)
	{
		return;
	}

	editType = i;
	displayEditType();
}

void EditVelocityScreen::setValue(int i)
{
	if (i < 1 || i > 200)
	{
		return;
	}

	if (editType != 2 && i > 127)
	{
		return;
	}

	value = i;
	displayValue();
}

void EditVelocityScreen::displayValue()
{
	findField("value")->setText(std::to_string(value));
}

void EditVelocityScreen::displayEditType()
{
	findField("edittype")->setText(editTypeNames[editType]);
}
