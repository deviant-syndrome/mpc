#pragma once
#include <lcdgui/ScreenComponent.hpp>

namespace mpc::lcdgui::screens::dialog
{
	class ConvertSoundScreen
		: public mpc::lcdgui::ScreenComponent
	{

	public:
		void turnWheel(int i) override;
		void function(int i) override;

		ConvertSoundScreen(const int layerIndex);

		void open() override;

	private:
		const std::vector<std::string> convertNames = std::vector<std::string>{ "STEREO TO MONO", "RE-SAMPLE" };
		void displayConvert();

	};
}
