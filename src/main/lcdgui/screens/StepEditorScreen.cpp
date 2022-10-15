#include "StepEditorScreen.hpp"

#include <audiomidi/EventHandler.hpp>
#include <audiomidi/AudioMidiServices.hpp>

#include <hardware/Hardware.hpp>
#include <hardware/HwPad.hpp>

#include <lcdgui/screens/window/TimingCorrectScreen.hpp>
#include <lcdgui/screens/window/EditMultipleScreen.hpp>

#include <lcdgui/Rectangle.hpp>

#include <sequencer/ChannelPressureEvent.hpp>
#include <sequencer/ControlChangeEvent.hpp>
#include <sequencer/MixerEvent.hpp>
#include <sequencer/Track.hpp>
#include <sequencer/NoteEvent.hpp>
#include <sequencer/PitchBendEvent.hpp>
#include <sequencer/PolyPressureEvent.hpp>
#include <sequencer/ProgramChangeEvent.hpp>
#include <sequencer/SystemExclusiveEvent.hpp>
#include <sequencer/SeqUtil.hpp>

#include <Util.hpp>

#include <mpc/MpcMultiMidiSynth.hpp>
#include <midi/core/ShortMessage.hpp>
#include <audio/server/NonRealTimeAudioServer.hpp>

#include <stdexcept>

using namespace mpc::lcdgui;
using namespace mpc::lcdgui::screens::window;
using namespace mpc::lcdgui::screens;
using namespace mpc::sequencer;
using namespace moduru::lang;
using namespace std;

const int EVENT_ROW_COUNT = 4;

StepEditorScreen::StepEditorScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "step-editor", layerIndex)
{
	lastColumn["empty"] = "a";
	lastColumn["channel-pressure"] = "a";
	lastColumn["control-change"] = "a";
	lastColumn["midi-clock"] = "a";
	lastColumn["mixer"] = "a";
	lastColumn["note"] = "a";
	lastColumn["pitch-bend"] = "a";
	lastColumn["poly-pressure"] = "a";
	lastColumn["program-change"] = "a";
	lastColumn["system-exclusive"] = "a";
	lastColumn["tempo-change"] = "a";

	for (int i = 0; i < EVENT_ROW_COUNT; i++)
		addChild(make_shared<EventRow>(mpc, i)).lock();

	MRECT r(31, 0, 164, 9);
	addChildT<Rectangle>("view-background", r);
}

void StepEditorScreen::openWindow()
{
    openScreen("step-edit-options");
}


void StepEditorScreen::open()
{
	findField("tonote").lock()->setLocation(115, 0);
	findLabel("fromnote").lock()->Hide(true);
	auto rectangle = findChild<Rectangle>("view-background").lock();

	lastRow = 0;

	init();

	if (track.lock()->getBus() != 0)
	{
		int pgm = sampler->getDrumBusProgramNumber(track.lock()->getBus());
		program = sampler->getProgram(pgm);
		findField("fromnote").lock()->setAlignment(Alignment::None);
	}
	else
	{
		findField("fromnote").lock()->setAlignment(Alignment::Centered, 18);
		findField("tonote").lock()->setAlignment(Alignment::Centered, 18);

		if (lastColumn["note"].compare("e") == 0)
			lastColumn["note"] = "c";
	}

	updateComponents();
	setViewNotesText();
	displayView();
	sequencer->addObserver(this);
	track.lock()->addObserver(this);

	findField("now0").lock()->setTextPadded(sequencer->getCurrentBarIndex() + 1, "0");
	findField("now1").lock()->setTextPadded(sequencer->getCurrentBeatIndex() + 1, "0");
	findField("now2").lock()->setTextPadded(sequencer->getCurrentClockNumber(), "0");

	initVisibleEvents();

	refreshEventRows();
	refreshSelection();

	auto previousScreen = ls.lock()->getPreviousScreenName();

	if (previousScreen.compare("step-timing-correct") != 0 &&
		previousScreen.compare("insert-event") != 0 &&
		previousScreen.compare("paste-event") != 0)
	{
		auto eventType = visibleEvents[0].lock()->getTypeName();
		ls.lock()->setFocus(lastColumn[eventType] + "0");
	}
}

void StepEditorScreen::close()
{
	init();
	sequencer->deleteObserver(this);
	track.lock()->deleteObserver(this);

	if (param.length() == 2)
	{
		int srcNumber = stoi(param.substr(1, 2));
		auto srcLetter = param.substr(0, 1);
		auto eventType = visibleEvents[srcNumber].lock()->getTypeName();
		lastColumn[eventType] = srcLetter;
	}

	auto nextScreen = ls.lock()->getCurrentScreenName();

	if (nextScreen.compare("step-timing-correct") != 0 &&
		nextScreen.compare("insert-event") != 0 &&
		nextScreen.compare("paste-event") != 0)
	{
		track.lock()->removeDoubles();
	}

	for (auto& e : visibleEvents)
	{
		if (e.lock())
			e.lock()->deleteObserver(this);
	}

	for (auto& e : eventsAtCurrentTick)
		if (e.lock())
			e.lock()->deleteObserver(this);

	clearSelection();
}

void StepEditorScreen::function(int i)
{
	init();

	switch (i)
	{
	case 0:
		openScreen("step-timing-correct");
		break;
	case 1:
		if (selectionStartIndex != -1)
		{
			// CopySelectedNotes
			setSelectedEvents();
			placeHolder = selectedEvents;
			clearSelection();
		}
		else if (selectionStartIndex == -1 && param.length() == 2)
		{
			// CopySelectedNote
			auto eventIndex = stoi(param.substr(1, 1));
			auto maybeEmptyEvent = dynamic_pointer_cast<EmptyEvent>(visibleEvents[eventIndex].lock());

			if (!maybeEmptyEvent)
				placeHolder = { visibleEvents[eventIndex].lock() };
		}
		break;
	case 2:
	{
		if (param.length() != 2)
			return;

		auto rowIndex = stoi(param.substr(1, 2));

		if (selectionStartIndex != -1)
		{
			removeEvents();
			ls.lock()->setFocus("a0");
			return;
		}

		if (!dynamic_pointer_cast<EmptyEvent>(visibleEvents[rowIndex].lock()))
		{
			for (int e = 0; e < track.lock()->getEvents().size(); e++)
			{
				if (track.lock()->getEvents()[e].lock() == visibleEvents[rowIndex].lock())
				{
					track.lock()->removeEvent(e);
					break;
				}
			}

			if (rowIndex == 2 && yOffset > 0)
				yOffset--;
		}

		initVisibleEvents();
		refreshEventRows();
		refreshSelection();

		auto eventType = visibleEvents[rowIndex].lock()->getTypeName();

		ls.lock()->setFocus(lastColumn[eventType] + to_string(rowIndex));
		break;
	}
	case 3:
	{
		bool posIsLastTick = sequencer->getTickPosition() == sequencer->getActiveSequence().lock()->getLastTick();

		if (selectionEndIndex == -1)
		{
			if (!posIsLastTick)
				openScreen("insert-event");
		}
		else
		{
			auto row = stoi(param.substr(1, 2));

			auto event = visibleEvents[row].lock();

			auto pitchEvent = dynamic_pointer_cast<PitchBendEvent>(event);
			auto mixerEvent = dynamic_pointer_cast<MixerEvent>(event);
			auto sysexEvent = dynamic_pointer_cast<SystemExclusiveEvent>(event);
			auto maybeEmptyEvent = dynamic_pointer_cast<EmptyEvent>(event);

			if (pitchEvent || mixerEvent || sysexEvent || maybeEmptyEvent)
				return;

			auto noteEvent = dynamic_pointer_cast<NoteEvent>(event);
			auto pgmChangeEvent = dynamic_pointer_cast<ProgramChangeEvent>(event);
			auto chPressEvent = dynamic_pointer_cast<ChannelPressureEvent>(event);
			auto polyPressEvent = dynamic_pointer_cast<PolyPressureEvent>(event);
			auto controlChangeEvent = dynamic_pointer_cast<ControlChangeEvent>(event);

			auto column = param.substr(0, 1);

			bool isA = column.compare("a") == 0;
			bool isB = column.compare("b") == 0;
			bool isC = column.compare("c") == 0;
			bool isD = column.compare("d") == 0;
			bool isE = column.compare("e") == 0;

			if ((polyPressEvent || controlChangeEvent) && isA)
				return;

			auto editMultipleScreen = mpc.screens->get<EditMultipleScreen>("edit-multiple");

			if (noteEvent && track.lock()->getBus() != 0)
			{
				if (isA)
				{
					editMultipleScreen->setChangeNoteTo(noteEvent->getNote());
				}
				else if (isB)
				{
					editMultipleScreen->setVariationType(noteEvent->getVariationType());
				}
				else if (isC)
				{
					editMultipleScreen->setVariationType(noteEvent->getVariationType());
					editMultipleScreen->seVariationValue(noteEvent->getVariationValue());
				}
				else if (isD)
				{
					editMultipleScreen->setEditValue(noteEvent->getDuration());
				}
				else if (isE)
				{
					editMultipleScreen->setEditValue(noteEvent->getVelocity());
				}
			}

			if (noteEvent && track.lock()->getBus() == 0)
			{
				if (isA)
					editMultipleScreen->setChangeNoteTo(noteEvent->getNote());
				else if (isB)
					editMultipleScreen->setEditValue(noteEvent->getDuration());
				else if (isC)
					editMultipleScreen->setEditValue(noteEvent->getVelocity());
			}
			else if (pgmChangeEvent)
			{
				editMultipleScreen->setEditValue(0);
			}
			else if (chPressEvent)
			{
				editMultipleScreen->setEditValue(chPressEvent->getAmount());
			}
			else if (polyPressEvent)
			{
				editMultipleScreen->setEditValue(polyPressEvent->getAmount());
			}
			else if (controlChangeEvent)
			{
				editMultipleScreen->setEditValue(controlChangeEvent->getAmount());
			}

			setSelectedEvent(visibleEvents[row]);
			setSelectedEvents();
			setSelectedParameterLetter(column);
			openScreen("edit-multiple");
		}

		break;
	}
	case 4:
		if (placeHolder.size() != 0)
			openScreen("paste-event");
		break;
	case 5:
		if (selectionStartIndex == -1)
		{
			if (param.length() == 2)
			{
				auto eventNumber = stoi(param.substr(1, 2));
				auto event = visibleEvents[eventNumber].lock();
				auto noteEvent = dynamic_pointer_cast<NoteEvent>(event);

				if (noteEvent)
                {
                    adhocPlayNoteEvent(noteEvent);
                }
			}
		}
		else
		{
			clearSelection();
		}
		break;
	}
}

void StepEditorScreen::turnWheel(int i)
{
	init();

	if (param.compare("view") == 0)
	{
		setView(view + i);
	}
	else if (param.compare("now0") == 0)
	{
		sequencer->setBar(sequencer->getCurrentBarIndex() + i);
	}
	else if (param.compare("now1") == 0)
	{
		sequencer->setBeat(sequencer->getCurrentBeatIndex() + i);
	}
	else if (param.compare("now2") == 0)
	{
		sequencer->setClock(sequencer->getCurrentClockNumber() + i);
	}
	else if (param.compare("tcvalue") == 0)
	{
		auto screen = mpc.screens->get<TimingCorrectScreen>("timing-correct");
		auto noteValue = screen->getNoteValue();
		screen->setNoteValue(noteValue + i);
	}
	else if (param.compare("fromnote") == 0 && view == 1)
	{
		if (track.lock()->getBus() != 0) setFromNote(fromNote + i);
		if (track.lock()->getBus() == 0) setNoteA(noteA + i);
	}
	else if (param.compare("tonote") == 0)
	{
		setNoteB(noteB + i);
	}
	else if (param.compare("fromnote") == 0 && view == 3)
	{
		setControl(control + i);
	}
	else if (param.length() == 2)
	{
		auto eventNumber = stoi(param.substr(1, 2));

		auto sysEx = dynamic_pointer_cast<SystemExclusiveEvent>(visibleEvents[eventNumber].lock());
		auto channelPressure = dynamic_pointer_cast<ChannelPressureEvent>(visibleEvents[eventNumber].lock());
		auto polyPressure = dynamic_pointer_cast<PolyPressureEvent>(visibleEvents[eventNumber].lock());
		auto controlChange = dynamic_pointer_cast<ControlChangeEvent>(visibleEvents[eventNumber].lock());
		auto programChange = dynamic_pointer_cast<ProgramChangeEvent>(visibleEvents[eventNumber].lock());
		auto pitchBend = dynamic_pointer_cast<PitchBendEvent>(visibleEvents[eventNumber].lock());
		auto mixer = dynamic_pointer_cast<MixerEvent>(visibleEvents[eventNumber].lock());
		auto note = dynamic_pointer_cast<NoteEvent>(visibleEvents[eventNumber].lock());

		if (sysEx)
		{
			if (param.find("a") != string::npos)
				sysEx->setByteA(sysEx->getByteA() + i);
			else if (param.find("b") != string::npos)
				sysEx->setByteB(sysEx->getByteB() + i);
		}
		else if (channelPressure)
		{
			if (param.find("a") != string::npos)
				channelPressure->setAmount(channelPressure->getAmount() + i);
		}
		else if (polyPressure)
		{
			if (param.find("a") != string::npos)
				polyPressure->setNote(polyPressure->getNote() + i);
			else if (param.find("b") != string::npos)
				polyPressure->setAmount(polyPressure->getAmount() + i);
		}
		else if (controlChange)
		{
			if (param.find("a") != string::npos)
				controlChange->setController(controlChange->getController() + i);
			else if (param.find("b") != string::npos)
				controlChange->setAmount(controlChange->getAmount() + i);
		}
		else if (programChange)
		{
			if (param.find("a") != string::npos)
				programChange->setProgram(programChange->getProgram() + i);
		}
		else if (pitchBend)
		{
			if (param.find("a") != string::npos)
				pitchBend->setAmount(pitchBend->getAmount() + i);
		}
		else if (mixer)
		{
			if (param.find("a") != string::npos)
				mixer->setParameter(mixer->getParameter() + i);
			else if (param.find("b") != string::npos)
				mixer->setPadNumber(mixer->getPad() + i);
			else if (param.find("c") != string::npos)
				mixer->setValue(mixer->getValue() + i);
		}
		else if (note && track.lock()->getBus() == 0)
		{
			if (param.find("a") != string::npos)
				note->setNote(note->getNote() + i);
			else if (param.find("b") != string::npos)
				note->setDuration(note->getDuration() + i);
			else if (param.find("c") != string::npos)
				note->setVelocity(note->getVelocity() + i);
		}
		else if (note && track.lock()->getBus() != 0)
		{
			if (param.find("a") != string::npos)
			{
				if (note->getNote() + i > 98)
				{
					if (note->getNote() != 98)
						note->setNote(98);

					return;
				}
				else if (note->getNote() + i < 35)
				{
					if (note->getNote() != 35)
						note->setNote(35);

					return;
				}
				else if (note->getNote() < 35)
				{
					note->setNote(35);
					return;
				}
				else if (note->getNote() > 98)
				{
					note->setNote(98);
					return;
				}

				note->setNote(note->getNote() + i);
			}
			else if (param.find("b") != string::npos)
			{
				note->setVariationTypeNumber(note->getVariationType() + i);
			}
			else if (param.find("c") != string::npos)
			{
				note->setVariationValue(note->getVariationValue() + i);
			}
			else if (param.find("d") != string::npos)
			{
				note->setDuration(note->getDuration() + i);
			}
			else if (param.find("e") != string::npos)
			{
				note->setVelocity(note->getVelocity() + i);
			}
		}
	}

    if (param.find("now") != std::string::npos)
    {
        adhocPlayNoteEventsAtCurrentPosition();
    }

	refreshSelection();
}

void StepEditorScreen::prevStepEvent()
{
	init();

	auto controls = mpc.getControls().lock();

	if (controls->isGoToPressed())
		sequencer->goToPreviousEvent();
	else
		sequencer->goToPreviousStep();

    adhocPlayNoteEventsAtCurrentPosition();
}

void StepEditorScreen::nextStepEvent()
{
	init();

	auto controls = mpc.getControls().lock();

	if (controls->isGoToPressed())
		sequencer->goToNextEvent();
	else
		sequencer->goToNextStep();

    adhocPlayNoteEventsAtCurrentPosition();
}

void StepEditorScreen::prevBarStart()
{
	init();
	auto controls = mpc.getControls().lock();

	if (controls->isGoToPressed())
		sequencer->setBar(0);
	else
		sequencer->setBar(sequencer->getCurrentBarIndex() - 1);

    adhocPlayNoteEventsAtCurrentPosition();
}

void StepEditorScreen::nextBarEnd()
{
	init();

	auto controls = mpc.getControls().lock();

	if (controls->isGoToPressed())
		sequencer->setBar(sequencer->getActiveSequence().lock()->getLastBarIndex() + 1);
	else
		sequencer->setBar(sequencer->getCurrentBarIndex() + 1);

    adhocPlayNoteEventsAtCurrentPosition();
}

void StepEditorScreen::left()
{
	init();

	if (param.length() == 2 && param.substr(0, 1).compare("a") == 0)
	{
		lastRow = stoi(param.substr(1, 2));
		ls.lock()->setFocus("view");
	}
	else
	{
		ScreenComponent::left();
	}

	checkSelection();
	refreshSelection();
}

void StepEditorScreen::right()
{
	ScreenComponent::right();
	checkSelection();
	refreshSelection();
}

void StepEditorScreen::up()
{
	init();

	if (param.length() == 2)
	{
		auto src = param;
		auto srcLetter = src.substr(0, 1);
		int srcNumber = stoi(src.substr(1, 2));
		auto controls = mpc.getControls().lock();

		if (controls->isShiftPressed() && selectionStartIndex == -1 && dynamic_pointer_cast<EmptyEvent>(visibleEvents[srcNumber].lock()))
			return;

		if (!controls->isShiftPressed() && srcNumber == 0 && yOffset == 0)
		{
			clearSelection();
			auto eventType = visibleEvents[srcNumber].lock()->getTypeName();
			lastColumn[eventType] = srcLetter;
			lastRow = 0;
			ls.lock()->setFocus("view");
			refreshSelection();
			return;
		}

		if (srcNumber == 0 && yOffset != 0)
		{
			auto oldEventType = visibleEvents[srcNumber].lock()->getTypeName();
			lastColumn[oldEventType] = srcLetter;

			setyOffset(yOffset - 1);

			auto newEventType = visibleEvents[srcNumber].lock()->getTypeName();

			ls.lock()->setFocus(lastColumn[newEventType] + to_string(srcNumber));

			if (controls->isShiftPressed())
				setSelectionEndIndex(srcNumber + yOffset);

			refreshSelection();
			return;
		}

		downOrUp(-1);
	}
}

void StepEditorScreen::down()
{
	init();

	if (param.compare("view") == 0 || param.find("now") != string::npos || param.compare("fromnote") == 0 || param.compare("tonote") == 0)
	{
		auto eventType = visibleEvents[lastRow].lock()->getTypeName();
		ls.lock()->setFocus(lastColumn[eventType] + to_string(lastRow));
		return;
	}

	if (param.length() == 2)
	{
		auto src = param;
		auto srcLetter = src.substr(0, 1);
		int srcNumber = stoi(src.substr(1, 2));
		auto controls = mpc.getControls().lock();

		if (srcNumber == 3)
		{
			if (yOffset + 4 == eventsAtCurrentTick.size())
			{
				return;
			}

			auto oldEventType = visibleEvents[srcNumber].lock()->getTypeName();
			lastColumn[oldEventType] = srcLetter;

			setyOffset(yOffset + 1);

			auto newEventType = visibleEvents[srcNumber].lock()->getTypeName();
			auto newColumn = lastColumn[newEventType];

			ls.lock()->setFocus(newColumn + "3");

			if (controls->isShiftPressed() && dynamic_pointer_cast<EmptyEvent>(visibleEvents[3].lock()))
				setSelectionEndIndex(srcNumber + yOffset);

			refreshSelection();
			return;
		}

		downOrUp(1);
	}
}

void StepEditorScreen::shift()
{
	init();
	ScreenComponent::shift();

	if (param.length() == 2)
	{
		auto eventNumber = stoi(param.substr(1, 2));
		setSelectionStartIndex(eventNumber + yOffset);
	}
}

void StepEditorScreen::downOrUp(int increment)
{
	if (param.length() == 2)
	{
		auto src = param;
		auto srcLetter = src.substr(0, 1);
		int srcNumber = stoi(src.substr(1, 2));
		auto controls = mpc.getControls().lock();
		auto destination = srcLetter + to_string(srcNumber + increment);

		if (srcNumber + increment != -1)
		{
			if (visibleEvents[srcNumber + increment].lock())
			{
				auto oldEventType = visibleEvents[srcNumber].lock()->getTypeName();
				lastColumn[oldEventType] = srcLetter;

				auto newEventType = visibleEvents[srcNumber + increment].lock()->getTypeName();
				ls.lock()->setFocus(lastColumn[newEventType] + to_string(srcNumber + increment));
			}
		}

		if (controls->isShiftPressed())
		{
			setSelectionEndIndex(srcNumber + increment + yOffset);
		}
		else
		{
			checkSelection();
			refreshSelection();
		}
	}
}

void StepEditorScreen::refreshSelection()
{
	auto firstEventIndex = min(selectionStartIndex, selectionEndIndex);
	auto lastEventIndex = max(selectionStartIndex, selectionEndIndex);

	bool somethingSelected = false;

	if (firstEventIndex != -1)
	{
		for (int i = 0; i < EVENT_ROW_COUNT; i++)
		{
			int absoluteEventNumber = i + yOffset;
			auto eventRow = findEventRows()[i].lock();

			if (absoluteEventNumber >= firstEventIndex && absoluteEventNumber < lastEventIndex + 1)
			{
				eventRow->setSelected(true);
				somethingSelected = true;
			}
			else
			{
				eventRow->setSelected(false);
			}
		}
	}
	else
	{
		for (int i = 0; i < EVENT_ROW_COUNT; i++)
		{
			auto eventRow = findEventRows()[i].lock();
			eventRow->setSelected(false);
		}
	}

	if (somethingSelected)
	{
		ls.lock()->setFunctionKeysArrangement(1);
	}
}

void StepEditorScreen::initVisibleEvents()
{
	init();

	for (auto& e : eventsAtCurrentTick)
		if (e.lock()) e.lock()->deleteObserver(this);

	eventsAtCurrentTick.clear();

	for (auto& event : track.lock()->getEvents())
	{
		auto lEvent = event.lock();

		if (lEvent->getTick() == sequencer->getTickPosition())
		{
			if ((view == 0
				|| view == 1)
				&& dynamic_pointer_cast<NoteEvent>(lEvent))
			{
				auto ne = dynamic_pointer_cast<NoteEvent>(lEvent);

				if (track.lock()->getBus() != 0)
				{
					if (fromNote == 34 || view == 0)
					{
						eventsAtCurrentTick.push_back(ne);
					}
					else if (fromNote != 34
						&& fromNote == ne->getNote())
					{
						eventsAtCurrentTick.push_back(ne);
					}
				}
				else
				{
					if ( (ne->getNote() >= noteA
						&& ne->getNote() <= noteB)
						|| view == 0)
					{
						eventsAtCurrentTick.push_back(ne);
					}
				}
			}

			if ((view == 0
				|| view == 2)
				&& dynamic_pointer_cast<PitchBendEvent>(lEvent))
			{
				eventsAtCurrentTick.push_back(event);
			}

			if ((view == 0 || view == 3)
				&& dynamic_pointer_cast<ControlChangeEvent>(lEvent))
			{
				if (control == -1) {
					eventsAtCurrentTick.push_back(event);
				}
				if (control == dynamic_pointer_cast<ControlChangeEvent>(lEvent)->getController())
				{
					eventsAtCurrentTick.push_back(event);
				}
			}
			if ((view == 0
				|| view == 4)
				&& dynamic_pointer_cast<ProgramChangeEvent>(lEvent))
			{
				eventsAtCurrentTick.push_back(event);
			}

			if ((view == 0
				|| view == 5)
				&& dynamic_pointer_cast<ChannelPressureEvent>(lEvent))
			{
				eventsAtCurrentTick.push_back(event);
			}

			if ((view == 0
				|| view == 6)
				&& dynamic_pointer_cast<PolyPressureEvent>(lEvent))
			{
				eventsAtCurrentTick.push_back(event);
			}

			if ((view == 0
				|| view == 7)
				&& (dynamic_pointer_cast<SystemExclusiveEvent>(lEvent)
					|| dynamic_pointer_cast<MixerEvent>(lEvent)))
			{
				eventsAtCurrentTick.push_back(event);
			}

		}
	}

	eventsAtCurrentTick.push_back(emptyEvent);

	for (auto& e : visibleEvents)
	{
		if (e.lock())
		{
			e.lock()->deleteObserver(this);
		}
	}

	visibleEvents = vector<weak_ptr<Event>>(4);
	int firstVisibleEventIndex = yOffset;
	int visibleEventCounter = 0;

	for (int i = 0; i < EVENT_ROW_COUNT; i++)
	{
		visibleEvents[visibleEventCounter] = eventsAtCurrentTick[i + firstVisibleEventIndex];
		visibleEventCounter++;

		if (visibleEventCounter > 3 || visibleEventCounter > eventsAtCurrentTick.size() - 1)
		{
			break;
		}
	}
}

void StepEditorScreen::refreshEventRows()
{
	for (int i = 0; i < EVENT_ROW_COUNT; i++)
	{
		auto eventRow = findEventRows()[i].lock();
		auto event = visibleEvents[i].lock();

		if (event)
		{
			eventRow->Hide(false);
			event->addObserver(this);
			eventRow->setBus(sequencer->getActiveTrack().lock()->getBus());
		}
		else
		{
			eventRow->Hide(true);
		}

		eventRow->setEvent(event);
		eventRow->init();
	}
}

void StepEditorScreen::updateComponents()
{
	init();

	if (view == 1 && track.lock()->getBus() != 0)
	{
		findField("fromnote").lock()->Hide(false);
		findField("fromnote").lock()->setSize(37, 9);
		findField("fromnote").lock()->setLocation(67, 0);
		findLabel("tonote").lock()->Hide(true);
		findField("tonote").lock()->Hide(true);
	}
	else if (view == 1 && track.lock()->getBus() == 0)
	{
		findField("fromnote").lock()->Hide(false);
		findField("fromnote").lock()->setLocation(61, 0);
		findField("fromnote").lock()->setSize(47, 9);
		findField("tonote").lock()->setSize(47, 9);
		findLabel("tonote").lock()->Hide(false);
		findLabel("tonote").lock()->setText("-");
		findField("tonote").lock()->Hide(false);
	}
	else if (view == 3)
	{
		auto fromNoteField = findField("fromnote").lock();
		fromNoteField->Hide(false);
		fromNoteField->setLocation(60, 0);
		fromNoteField->setSize(104, 9);

		findLabel("tonote").lock()->Hide(true);
		findField("tonote").lock()->Hide(true);
	}
	else if (view != 1 && view != 3)
	{
		findField("fromnote").lock()->Hide(true);
		findLabel("tonote").lock()->Hide(true);
		findField("tonote").lock()->Hide(true);
	}
}

void StepEditorScreen::setViewNotesText()
{
	init();

	if (view == 1 && track.lock()->getBus() != 0)
	{
		if (fromNote == 34) {
            findField("fromnote").lock()->setText("ALL");
        }
		else
        {
            auto padName = sampler->getPadName(program.lock()->getPadIndexFromNote(fromNote));
			findField("fromnote").lock()->setText(to_string(fromNote) + "/" + padName);
        }
	}
	else if (view == 1 && track.lock()->getBus() == 0)
	{
		findField("fromnote").lock()->setText(StrUtil::padLeft(to_string(noteA), " ", 3) + "(" + mpc::Util::noteNames()[noteA] + u8"\u00D4");
		findField("tonote").lock()->setText(StrUtil::padLeft(to_string(noteB), " ", 3) + "(" + mpc::Util::noteNames()[noteB] + u8"\u00D4");
	}
	else if (view == 3)
	{
		if (control == -1)
			findField("fromnote").lock()->setText("   -    ALL");
		else
			findField("fromnote").lock()->setText(StrUtil::padLeft(to_string(control), " ", 3) + "-" + EventRow::controlNames[control]);
	}

	findField("view").lock()->setText(viewNames[view]);
	auto newWidth = findField("view").lock()->getText().length() * 6 + 1;
	findField("view").lock()->setSize(newWidth, 9);
}

void StepEditorScreen::setView(int i)
{
	if (i < 0 || i > 7)
		return;

	view = i;

	displayView();
	updateComponents();
	setViewNotesText();
	setyOffset(0);
	findChild<Rectangle>().lock()->SetDirty();
}

void StepEditorScreen::setNoteA(int i)
{
	if (i < 0 || i > 127)
		return;

	noteA = i;

	if (noteA > noteB)
		noteB = noteA;

	setViewNotesText();
	initVisibleEvents();
	refreshEventRows();
	refreshSelection();
}

void StepEditorScreen::setNoteB(int i)
{
	if (i < 0 || i > 127)
		return;

	noteB = i;

	if (noteB < noteA)
		noteA = noteB;

	setViewNotesText();
	initVisibleEvents();
	refreshEventRows();
	refreshSelection();
}

void StepEditorScreen::setControl(int i)
{
	if (i < -1 || i > 127)
		return;

	control = i;

	setViewNotesText();
	initVisibleEvents();
	refreshEventRows();
	refreshSelection();
}

void StepEditorScreen::setyOffset(int i)
{
	if (i < 0)
		return;

	yOffset = i;

	initVisibleEvents();
	refreshEventRows();
	refreshSelection();
}

void StepEditorScreen::setFromNote(int i)
{
	if (i < 34 || i > 98)
		return;

	fromNote = i;

	setViewNotesText();
	displayView();
	updateComponents();
	setViewNotesText();
	initVisibleEvents();
	refreshEventRows();
	refreshSelection();
}

void StepEditorScreen::setSelectionStartIndex(int i)
{
	if (dynamic_pointer_cast<EmptyEvent>(eventsAtCurrentTick[i].lock()))
		return;

	selectionStartIndex = i;
	selectionEndIndex = i;

	ls.lock()->setFunctionKeysArrangement(1);
	refreshSelection();
}

void StepEditorScreen::clearSelection()
{
	selectionStartIndex = -1;
	selectionEndIndex = -1;
	ls.lock()->setFunctionKeysArrangement(0);
	refreshSelection();
}

void StepEditorScreen::setSelectionEndIndex(int i)
{
	if (i == -1)
		return;

	selectionEndIndex = i;
	refreshSelection();
}

void StepEditorScreen::finalizeSelection(int i)
{
	selectionEndIndex = i;

	if (selectionEndIndex < selectionStartIndex)
	{
		auto oldSelectionStartIndex = selectionStartIndex;
		selectionStartIndex = selectionEndIndex;
		selectionEndIndex = oldSelectionStartIndex;
	}

	refreshSelection();
}

void StepEditorScreen::setSelectedEvents()
{
	selectedEvents.clear();
	auto firstEventIndex = selectionStartIndex;
	auto lastEventIndex = selectionEndIndex;

	if (firstEventIndex > lastEventIndex)
	{
		firstEventIndex = selectionEndIndex;
		lastEventIndex = selectionStartIndex;
	}

	for (int i = firstEventIndex; i < lastEventIndex + 1; i++)
		selectedEvents.push_back(eventsAtCurrentTick[i].lock());
}

void StepEditorScreen::checkSelection()
{
	string focus = ls.lock()->getFocus();

	if (focus.length() != 2)
	{
		clearSelection();
		return;
	}

	int row = stoi(focus.substr(1, 2));
	int eventIndex = row + yOffset;

	if (eventIndex < selectionStartIndex || eventIndex > selectionEndIndex)
		clearSelection();
}

void StepEditorScreen::setSelectedEvent(weak_ptr<Event> event)
{
	selectedEvent = event.lock();
}

void StepEditorScreen::setSelectedParameterLetter(string str)
{
	selectedParameterLetter = str;
}


void StepEditorScreen::removeEvents()
{
	init();
	auto firstEventIndex = selectionStartIndex;
	auto lastEventIndex = selectionEndIndex;

	if (firstEventIndex > lastEventIndex)
	{
		firstEventIndex = selectionEndIndex;
		lastEventIndex = selectionStartIndex;
	}

	int eventCounter = 0;

	vector<weak_ptr<Event>> tempList;

	for (auto& e : eventsAtCurrentTick)
		tempList.push_back(e);

	for (auto& event : tempList)
	{
		if (eventCounter >= firstEventIndex && eventCounter <= lastEventIndex)
		{
			if (!dynamic_pointer_cast<EmptyEvent>(event.lock()))
				track.lock()->removeEvent(event);
		}
		eventCounter++;
	}

	clearSelection();
	setyOffset(0);
}

void StepEditorScreen::displayView()
{
	findField("view").lock()->setText(viewNames[view]);
}

void StepEditorScreen::update(moduru::observer::Observable*, nonstd::any message)
{
	auto msg = nonstd::any_cast<string>(message);

    if (msg.compare("step-editor") == 0)
	{
        auto& pads = mpc.getHardware().lock()->getPads();

        auto anyPadIsPressed = std::any_of(
                pads.begin(),
                pads.end(),
                [](const std::shared_ptr<mpc::hardware::HwPad> &p) {
                    return p->isPressed();
                });

        if (anyPadIsPressed)
		{
			// a note is currently being recorded by the user pressing a pad
			initVisibleEvents();
			refreshEventRows();
			return;
		}

		int eventNumber;

		try
		{
			init();
			eventNumber = stoi(param.substr(1, 2));
		}
		catch (const invalid_argument& e)
		{
			return;
		}

		auto eventRow = findEventRows()[eventNumber].lock();

		if (dynamic_pointer_cast<NoteEvent>(visibleEvents[eventNumber].lock()))
		{
			if (track.lock()->getBus() != 0)
				eventRow->setDrumNoteEventValues();
			else
				eventRow->setMidiNoteEventValues();
		}
		else if (dynamic_pointer_cast<MixerEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setMixerEventValues();
		}
		else if (dynamic_pointer_cast<PitchBendEvent>(visibleEvents[eventNumber].lock())
			|| dynamic_pointer_cast<ProgramChangeEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setMiscEventValues();
		}
		else if (dynamic_pointer_cast<ControlChangeEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setControlChangeEventValues();
		}
		else if (dynamic_pointer_cast<ChannelPressureEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setChannelPressureEventValues();
		}
		else if (dynamic_pointer_cast<PolyPressureEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setPolyPressureEventValues();
		}
		else if (dynamic_pointer_cast<SystemExclusiveEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setSystemExclusiveEventValues();
		}
		else if (dynamic_pointer_cast<EmptyEvent>(visibleEvents[eventNumber].lock()))
		{
			eventRow->setEmptyEventValues();
		}
	}
	else if (msg.compare("adjust-duration") == 0)
	{
		initVisibleEvents();
		refreshEventRows();
	}
	else if (msg.compare("bar") == 0)
	{
		if (sequencer->isPlaying())
			return;

		findField("now0").lock()->setTextPadded(sequencer->getCurrentBarIndex() + 1, "0");
		setyOffset(0);
	}
	else if (msg.compare("beat") == 0)
	{
		if (sequencer->isPlaying())
			return;

		findField("now1").lock()->setTextPadded(sequencer->getCurrentBeatIndex() + 1, "0");
		setyOffset(0);
	}
	else if (msg.compare("clock") == 0)
	{
		if (sequencer->isPlaying())
			return;

		findField("now2").lock()->setTextPadded(sequencer->getCurrentClockNumber(), "0");
		setyOffset(0);
	}
}

vector<weak_ptr<Event>>& StepEditorScreen::getVisibleEvents()
{
	return visibleEvents;
}

vector<shared_ptr<Event>>& StepEditorScreen::getSelectedEvents()
{
	return selectedEvents;
}

string StepEditorScreen::getSelectedParameterLetter()
{
	return selectedParameterLetter;
}

shared_ptr<Event> StepEditorScreen::getSelectedEvent()
{
	return selectedEvent;
}

vector<shared_ptr<Event>>& StepEditorScreen::getPlaceHolder()
{
	return placeHolder;
}

int StepEditorScreen::getYOffset()
{
	return yOffset;
}

vector<weak_ptr<EventRow>> StepEditorScreen::findEventRows()
{
	vector<weak_ptr<EventRow>> result;

	for (int i = 0; i < 4; i++)
		result.push_back(dynamic_pointer_cast<EventRow>(findChild("event-row-" + to_string(i)).lock()));

	return result;
}

void screens::StepEditorScreen::adhocPlayNoteEvent(const shared_ptr<mpc::sequencer::NoteEvent> &noteEvent)
{
    auto tick = noteEvent->getTick();
    noteEvent->setTick(-1);
    auto eventHandler = mpc.getEventHandler().lock();
    auto tr = track.lock().get();

    MidiAdapter midiAdapter;

    auto mms = mpc.getMms();

    midiAdapter.process(noteEvent, tr->getBus() - 1, noteEvent ? noteEvent->getVelocity() : 0);

    auto varType = noteEvent->getVariationType();
    auto varValue = noteEvent->getVariationValue();

    int uniqueEnoughID = playSingleEventCounter++;

    if (playSingleEventCounter < 0) playSingleEventCounter = 0;

    mms->mpcTransport(midiAdapter.get().lock().get(), 0, varType, varValue, 0, uniqueEnoughID);

    noteEvent->setTick(tick);

    auto frameSeq = mpc.getAudioMidiServices().lock()->getFrameSequencer().lock();
    auto sampleRate = mpc.getAudioMidiServices().lock()->getAudioServer()->getSampleRate();
    auto tempo = mpc.getSequencer().lock()->getTempo();
    auto &events = frameSeq->eventsAfterNFrames;

    for (auto &e: events)
    {
        if (!e.occupied.load())
        {
            auto durationInFrames = mpc::sequencer::SeqUtil::ticksToFrames(noteEvent->getDuration(),
                                                                           tempo, sampleRate);
            e.init(durationInFrames, [noteEvent, tr, mms, uniqueEnoughID]() {
                auto noteOff = noteEvent->getNoteOff().lock();
                auto noteOffTick = noteOff->getTick();
                noteOff->setTick(-1);
                noteOff->setNote(noteEvent->getNote());
                MidiAdapter midiAdapter2;
                midiAdapter2.process(noteOff, tr->getBus() - 1, 0);
                auto noteOffToSend = midiAdapter2.get();
                noteOff->setTick(noteOffTick);
                mms->mpcTransport(noteOffToSend.lock().get(), 0, 0, 0, 0, uniqueEnoughID);
            });
            break;
        }
    }
}

void screens::StepEditorScreen::adhocPlayNoteEventsAtCurrentPosition()
{
    auto tick = sequencer->getTickPosition();
    for (auto& e : track.lock()->getEventRange(tick, tick))
    {
        auto noteEvent = std::dynamic_pointer_cast<NoteEvent>(e.lock());
        if (noteEvent)
        {
            adhocPlayNoteEvent(noteEvent);
        }
    }
}
