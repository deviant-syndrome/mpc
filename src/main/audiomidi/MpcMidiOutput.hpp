#pragma once

#include <midi/core/ShortMessage.hpp>
#include <vector>

#include "thirdp/concurrentqueue.h"

namespace mpc::audiomidi {

	class MpcMidiOutput
	{

	private:
        moodycamel::ConcurrentQueue<std::shared_ptr<ctoot::midi::core::ShortMessage>> outputQueueA;
        moodycamel::ConcurrentQueue<std::shared_ptr<ctoot::midi::core::ShortMessage>> outputQueueB;

	public:
        void enqueMessageOutputA(std::shared_ptr<ctoot::midi::core::ShortMessage>);
        void enqueMessageOutputB(std::shared_ptr<ctoot::midi::core::ShortMessage>);
        unsigned char dequeueOutputA(std::vector<std::shared_ptr<ctoot::midi::core::ShortMessage>>& buf);
        unsigned char dequeueOutputB(std::vector<std::shared_ptr<ctoot::midi::core::ShortMessage>>& buf);
		void panic();

	};
}