#include <sequencer/MixerEvent.hpp>

using namespace mpc::sequencer;

void MixerEvent::setParameter(int i)
{
	if (i < 0 || i > 3) return;
	mixerParameter = i;
	
	notifyObservers(std::string("step-editor"));
}

int MixerEvent::getParameter()
{
    return mixerParameter;
}

void MixerEvent::setPadNumber(int i)
{
    if(i < 0 || i > 63) return;

    padNumber = i;
    
    notifyObservers(std::string("step-editor"));
}

int MixerEvent::getPad()
{
    return padNumber;
}

void MixerEvent::setValue(int i)
{
    if(i < 0 || i > 100) return;

    mixerParameterValue = i;
    
    notifyObservers(std::string("step-editor"));
}

int MixerEvent::getValue()
{
    return mixerParameterValue;
}

void MixerEvent::CopyValuesTo(std::weak_ptr<Event> dest) {
	Event::CopyValuesTo(dest);
	auto lDest = std::dynamic_pointer_cast<MixerEvent>(dest.lock());
	lDest->setPadNumber(getPad());
	lDest->setParameter(getParameter());
	lDest->setValue(getValue());
}
