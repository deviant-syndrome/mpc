#pragma once
#include <lcdgui/ScreenComponent.hpp>

namespace mpc::lcdgui::screens::window {

	class TrackScreen
		: public mpc::lcdgui::ScreenComponent
	{

	public:
		void function(int i) override;
		void turnWheel(int nameScreenName) override;

	public:
		void open() override;

	public:
		TrackScreen(mpc::Mpc& mpc, const int layerIndex);

	};
}
