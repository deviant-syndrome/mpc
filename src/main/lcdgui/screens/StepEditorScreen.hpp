#pragma once
#include <lcdgui/ScreenComponent.hpp>

#include <ui/sequencer/EventRow.hpp>
#include <sequencer/Event.hpp>
#include <sequencer/EmptyEvent.hpp>

#include <vector>
#include <memory>

namespace mpc::lcdgui::screens
{
	class StepEditorScreen
		: public mpc::lcdgui::ScreenComponent
	{

	private:
		void downOrUp(int increment);

	public:
		void function(int i) override;
		void turnWheel(int i) override;
		void prevStepEvent() override;
		void nextStepEvent() override;
		void prevBarStart() override;
		void nextBarEnd() override;
		void left() override;
		void right() override;
		void up() override;
		void down() override;
		void shift() override;

		StepEditorScreen(const int& layer);

		void open() override;
		void close() override;

		void update(moduru::observer::Observable*, nonstd::any message);

	private:
		void refreshSelection();
		void initVisibleEvents();

	private:
		void refreshEventRows();
		void refreshViewModeNotes();
		void setViewModeNotesText();

	private:
		std::vector<std::string> viewNames = { "ALL EVENTS", "NOTES", "PITCH BEND", "CTRL:", "PROG CHANGE", "CH PRESSURE", "POLY PRESS", "EXCLUSIVE" };
		std::shared_ptr<mpc::sequencer::EmptyEvent> emptyEvent = std::make_shared<mpc::sequencer::EmptyEvent>();
		std::vector<std::unique_ptr<mpc::ui::sequencer::EventRow>> eventRows;
		std::vector<std::weak_ptr<mpc::sequencer::Event>> visibleEvents;
		std::vector <std::weak_ptr<mpc::sequencer::Event>> eventsAtCurrentTick;
		std::vector<std::weak_ptr<mpc::sequencer::Event>> placeHolder;
		std::weak_ptr<mpc::sequencer::Event> selectedEvent;
		std::vector<std::weak_ptr<mpc::sequencer::Event>> selectedEvents;
		int viewModeNumber = 0;
		int noteA = 0;
		int noteB = 0;
		int controlNumber = -1;
		bool autoStepIncrementEnabled = false;
		bool durationOfRecordedNotes = false;
		int tcValueRecordedNotes = 100;
		int yOffset = 0;
		int selectedEventNumber = 0;
		int changeNoteToNumber = 35;
		int changeVariationTypeNumber = 0;
		int changeVariationValue = 0;
		int editValue = 0;
		int fromNotePad = 34;
		int selectionStartIndex = -1;
		int selectionEndIndex = -1;
		std::string selectedParameterLetter = "";
		bool durationTcPercentageEnabled = false;

	private:
		void setViewModeNumber(int i);
		void setNoteA(int i);
		void setNoteB(int i);
		void setControlNumber(int i);
		void setyOffset(int i);
		void setSelectedEventNumber(int i);
		void setFromNotePad(int i);
		void setSelectionStartIndex(int i);
		void setSelectedEvents();
		void setSelectedEvent(std::weak_ptr<mpc::sequencer::Event> event);
		void setSelectedParameterLetter(std::string str);

		void checkSelection();
		
		void removeEvents();

	private:
		void displayView();

	public:
		void setAutoStepIncrementEnabled(bool b);
		void setChangeNoteToNumber(int i);
		void setChangeVariationTypeNumber(int i);
		void setChangeVariationValue(int i);
		void setEditValue(int i);
		void setSelectionEndIndex(int i);
		void setDurationOfRecordedNotes(bool b);
		void setTcValueRecordedNotes(int i);

		bool isAutoStepIncrementEnabled();
		bool isDurationTcPercentageEnabled();
		int getTcValueRecordedNotes();

		std::vector<std::weak_ptr<mpc::sequencer::Event>>& getVisibleEvents();
		std::vector<std::weak_ptr<mpc::sequencer::Event>>& getSelectedEvents();
		std::weak_ptr<mpc::sequencer::Event> getSelectedEvent();
		std::string getSelectedParameterLetter();
		int getChangeVariationTypeNumber();
		int getChangeVariationValue();
		void clearSelection();
		int getChangeNoteToNumber();
		int getEditValue();
		int getYOffset();
		std::vector<std::weak_ptr<mpc::sequencer::Event>>& getPlaceHolder();

	};
}
