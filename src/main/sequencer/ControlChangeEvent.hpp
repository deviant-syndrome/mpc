#pragma once
#include <sequencer/Event.hpp>

#include <memory>

namespace mpc::sequencer
{
	class ControlChangeEvent
		: public Event
	{

	private:
		int controllerNumber = 0;
		int controllerValue = 0;

	public:
		void setController(int i);
		int getController();
		void setAmount(int i);
		int getAmount();

		void CopyValuesTo(std::weak_ptr<Event> dest) override;

		std::string getTypeName() override { return "control-change"; }

	};
}
