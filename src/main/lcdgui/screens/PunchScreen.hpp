#pragma once
#include <lcdgui/ScreenComponent.hpp>

#include <lcdgui/screens/WithTimesAndNotes.hpp>

namespace mpc::sequencer {
	class FrameSeq;
	class Track;
}

namespace mpc::lcdgui::screens {
	class SequencerScreen;
}

namespace mpc::lcdgui::screens {

	class PunchScreen
		: public mpc::lcdgui::ScreenComponent, private mpc::lcdgui::screens::WithTimesAndNotes
	{

	public:
		void turnWheel(int i) override;
		void function(int i) override;

	public:
		PunchScreen(mpc::Mpc& mpc, const int layerIndex);

		void open() override;

	private:
		const std::vector<std::string> autoPunchNames{ "PUNCH IN ONLY", "PUNCH OUT ONLY", "PUNCH IN OUT" };
		int autoPunch = 0;
		bool on = false;

		void setAutoPunch(int i);

		void displayAutoPunch();
		void displayBackground();

		void displayTime() override;
		void displayNotes() override {}

		friend class SequencerScreen;
		friend class mpc::sequencer::FrameSeq;
		friend class mpc::sequencer::Track;

	};
}
