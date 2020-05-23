#pragma once
#include <lcdgui/ScreenComponent.hpp>

namespace mpc::lcdgui::screens {

	class ZoneScreen
		: public mpc::lcdgui::ScreenComponent
	{

	public:


	public:
		void init() override;

	public:
		void openWindow() override;
		void function(int f) override;
		void turnWheel(int i) override;

		ZoneScreen(const int layerIndex);

	};
}
