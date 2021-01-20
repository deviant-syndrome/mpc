#ifndef KeyEventHandler_h
#define KeyEventHandler_h

#include <controls/KbMapping.hpp>

#include <memory>
#include <vector>

namespace mpc {
class Mpc;
}

namespace mpc::controls {
class KeyEvent;
}

namespace mpc::controls {

class KeyEventHandler {
  
public:
    KeyEventHandler(mpc::Mpc&);
    void handle(const KeyEvent& keyEvent);
    
private:
    mpc::Mpc& mpc;
    std::vector<int> pressed;
    std::shared_ptr<KbMapping> kbMapping;
    
};

}

#endif /* KeyEventHandler_h */
