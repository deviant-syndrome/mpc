#include "HwSlider.hpp"

#include <Mpc.hpp>
#include <lcdgui/ScreenComponent.hpp>

using namespace mpc::hardware;

Slider::Slider(mpc::Mpc& mpc)
	: mpc(mpc)
{
}

void Slider::setValue(int i)
{
	if (i < 0 || i > 127)
		return;

	value = i;
	
	if (mpc.getActiveControls())
		mpc.getActiveControls()->setSlider(value);

	notifyObservers(value);
}

int Slider::getValue() {
	return value;
}
