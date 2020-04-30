#pragma once
#include <controls/BaseControls.hpp>

#include <memory>

namespace mpc {
	
	namespace sequencer {
		class Sequence;
	}

	namespace ui {
		class UserDefaults;
		class NameGui;

		namespace sequencer {
			namespace window {
				class SequencerWindowGui;
			}
		}
		namespace disk {
			namespace window {
				class DirectoryGui;
			}
		}
	}

	namespace controls {
		namespace other {

			class AbstractOtherControls
				: public mpc::controls::BaseControls
			{

			public:
				typedef mpc::controls::BaseControls super;

			protected:
				mpc::ui::sequencer::window::SequencerWindowGui* swGui{ nullptr };
		//		std::weak_ptr<mpc::ui::UserDefaults> ud{};
				mpc::ui::NameGui* nameGui{ nullptr };
				mpc::ui::disk::window::DirectoryGui* directoryGui{ nullptr };
				std::weak_ptr<mpc::sequencer::Sequence> sequence{ };
				
			protected:
				std::vector<int> tickValues{};
				std::vector<char> akaiAsciiChar{};
				int trackNum{ 0 };
				int seqNum{ 0 };

			protected:
				void init() override;

			public:
				AbstractOtherControls();
				virtual ~AbstractOtherControls();

			};

		}
	}
}
