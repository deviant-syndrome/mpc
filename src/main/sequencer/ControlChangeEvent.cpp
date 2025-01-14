#include <sequencer/ControlChangeEvent.hpp>

using namespace mpc::sequencer;

void ControlChangeEvent::setController(int i)
{
	if (i < 0 || i > 127) return;
	controllerNumber = i;
	
	notifyObservers(std::string("step-editor"));
}

int ControlChangeEvent::getController()
{
    return controllerNumber;
}

void ControlChangeEvent::setAmount(int i)
{
    if(i < 0 || i > 127) return;

    controllerValue = i;
    
    notifyObservers(std::string("step-editor"));
}

int ControlChangeEvent::getAmount()
{
    return controllerValue;
}

void ControlChangeEvent::CopyValuesTo(std::weak_ptr<Event> dest) {
	auto lDest = std::dynamic_pointer_cast<ControlChangeEvent>(dest.lock());
	Event::CopyValuesTo(dest);
	lDest->setAmount(this->getAmount());
	lDest->setController(this->getController());
}
