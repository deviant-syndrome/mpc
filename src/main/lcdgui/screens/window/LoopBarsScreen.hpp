#pragma once
#include <lcdgui/ScreenComponent.hpp>

namespace mpc::lcdgui::screens::window {

	class LoopBarsScreen
		: public mpc::lcdgui::ScreenComponent
	{

	public:
		void turnWheel(int i) override;

	public:
		LoopBarsScreen(const int layerIndex);

	public:
		void open() override;

	private:
		void displayNumberOfBars();
		void displayLastBar();
		void displayFirstBar();

	};
}
