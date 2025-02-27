#pragma once
#include <lcdgui/ScreenComponent.hpp>

#include <functional>

namespace mpc::controls { class BaseControls; }

namespace mpc::lcdgui { class Underline;}

namespace mpc::lcdgui::screens::window
{
class NameScreen
: public mpc::lcdgui::ScreenComponent
{
    
public:
    NameScreen(mpc::Mpc& mpc, const int layerIndex);
    
    void open() override;
    void close() override;
    
    void left() override;
    void right() override;
    void turnWheel(int j) override;
    void function(int i) override;
    void pressEnter() override;
    void mainScreen() override;
    void numpad() {} // Block ways to leave the screen inadvertently

    void typeCharacter(char c);
    void backSpace();

    void initialize(std::string name, unsigned char nameLimit,
                    const std::function<void(std::string&)>& enterAction,
                    const std::string& cancelScreen,
                    const std::function<void()>& mainScreenAction = []{});

    std::string getNameWithoutSpaces();
    void setEditing(bool b);

    void setName(std::string newName);

private:
    std::function<void(std::string&)> enterAction = [](std::string&){};
    std::string cancelScreen;
    std::function<void()> mainScreenAction = [](){};

    void drawUnderline();
    void initEditColors();
    void displayName();
    std::weak_ptr<mpc::lcdgui::Underline> findUnderline();
    
    void setNameLimit(int i);
    void setName(std::string str, int i);

    void changeNameCharacter(int i, bool up);
    std::string nameScreenName = "";
    bool editing = false;
    int nameLimit = 0;
};
}
