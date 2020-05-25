#pragma once
#include <lcdgui/ScreenComponent.hpp>

namespace mpc::lcdgui::screens::dialog
{
	class MonoToStereoScreen
		: public mpc::lcdgui::ScreenComponent
	{

	public:
		void turnWheel(int i) override;
		void function(int j) override;

		MonoToStereoScreen(const int layerIndex);

		void open() override;

	private:
		void displayLSource();
		void displayRSource();
		void displayNewStName();

		std::string newStName;
		int rSource = 0;
		void setRSource(int i);
		void setNewStName(std::string s);

	};
}
