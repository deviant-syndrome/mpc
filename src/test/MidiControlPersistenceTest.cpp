#include "catch2/catch_test_macros.hpp"

#include "Mpc.hpp"
#include "Paths.hpp"
#include "hardware/Hardware.hpp"
#include "lcdgui/screens/VmpcMidiScreen.hpp"
#include "nvram/MidiControlPersistence.hpp"
#include "disk/AbstractDisk.hpp"

#include "file/File.hpp"

using namespace mpc::nvram;
using namespace mpc::lcdgui::screens;
using namespace moduru::file;

TEST_CASE("Initial state", "[midi-control-persistence]")
{
    MidiControlPersistence::deleteLastState();
    mpc::Mpc mpc;
    mpc.init(1, 5);

    REQUIRE(!MidiControlPersistence::presets.empty());

    mpc.getLayeredScreen()->openScreen("vmpc-midi");

    auto screen = mpc.screens->get<VmpcMidiScreen>("vmpc-midi");
    auto activePreset = screen->getActivePreset();

    REQUIRE(!activePreset->rows.empty());

    auto buttonLabels = mpc.getHardware()->getButtonLabels();

    for (auto &l: buttonLabels)
    {
        auto found = std::find_if(
                activePreset->rows.begin(),
                activePreset->rows.end(),
                [l](const MidiControlCommand &c) { return c.label == l; });

        REQUIRE(found != activePreset->rows.end());
    }
}

TEST_CASE("VmpcMidiScreen", "[midi-control-persistence]")
{
    MidiControlPersistence::deleteLastState();
    mpc::Mpc mpc;
    mpc.init(1, 5);

    mpc.getLayeredScreen()->openScreen("vmpc-midi");
    auto controls = mpc.getActiveControls();
    controls->turnWheel(1);
    controls->right();
    controls->turnWheel(2);
    controls->right();
    controls->turnWheel(3);
    controls->function(5);
    controls->mainScreen();
    mpc.getLayeredScreen()->openScreen("vmpc-midi");
    auto t1 = controls->findChild<mpc::lcdgui::Field>("type0")->getText();
    auto t2 = controls->findChild<mpc::lcdgui::Field>("channel0")->getText();
    auto t3 = controls->findChild<mpc::lcdgui::Field>("value0")->getText();
    REQUIRE(t1 == "Note");
    REQUIRE(t2 == "ch 2");
    REQUIRE(t3 == "  2");
}

TEST_CASE("Save and load a preset", "[midi-control-persistence]")
{
    MidiControlPersistence::deleteLastState();
    mpc::Mpc mpc;
    mpc.init(1, 5);

    mpc.getLayeredScreen()->openScreen("vmpc-midi");
    auto controls = mpc.getActiveControls();
    controls->turnWheel(1);
    controls->right();
    controls->turnWheel(2);
    controls->right();
    controls->turnWheel(3);
    controls->function(5);
    controls->mainScreen();
    mpc.getLayeredScreen()->openScreen("vmpc-midi");
    auto t1 = controls->findChild<mpc::lcdgui::Field>("type0")->getText();
    auto t2 = controls->findChild<mpc::lcdgui::Field>("channel0")->getText();
    auto t3 = controls->findChild<mpc::lcdgui::Field>("value0")->getText();
    REQUIRE(t1 == "Note");
    REQUIRE(t2 == "ch 2");
    REQUIRE(t3 == "  2");

    // Open VmpcMidiPresetsScreen
    controls->openWindow();

    controls = mpc.getActiveControls();

    // Open NameScreen
    controls->function(2);

    controls = mpc.getActiveControls();
    auto focus0 = mpc.getLayeredScreen()->getFocus();

    // Save the preset by pressing ENTER in NameScreen
    controls->function(4);

    File f(mpc::Paths::midiControlPresetsPath() + "New_preset.vmp", nullptr);
    REQUIRE(f.exists());
    auto preset = std::make_shared<MidiControlPreset>();
    mpc.getDisk()->readMidiControlPreset(f, preset);
    REQUIRE(preset->rows[0].label == "pad-1");
    REQUIRE(preset->rows[0].value == 2);
    REQUIRE(preset->rows[0].channel == 1);
    REQUIRE(preset->rows[0].isNote);

    mpc.getLayeredScreen()->openScreen("vmpc-midi");
    mpc.getLayeredScreen()->openScreen("vmpc-midi-presets");

    controls = mpc.getActiveControls();

    controls->down();
    controls->down();
    controls->right();

    // Set Auto Load to YES
    controls->turnWheel(2);

    f = File(mpc::Paths::midiControlPresetsPath() + "New_preset.vmp", nullptr);
    REQUIRE(f.exists());
    preset = std::make_shared<MidiControlPreset>();
    mpc.getDisk()->readMidiControlPreset(f, preset);

    REQUIRE(preset->autoloadMode == MidiControlPreset::AutoLoadMode::YES);
    REQUIRE(preset->rows[0].label == "pad-1");
    REQUIRE(preset->rows[0].value == 2);
    REQUIRE(preset->rows[0].channel == 1);
    REQUIRE(preset->rows[0].isNote);

    // Wait long enough for any Popup threads to have finished
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
}