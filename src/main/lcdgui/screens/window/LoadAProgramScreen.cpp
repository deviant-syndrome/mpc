#include "LoadAProgramScreen.hpp"

#include <disk/AbstractDisk.hpp>
#include <lcdgui/screens/LoadScreen.hpp>

#include "mpc/MpcSoundPlayerChannel.hpp"

using namespace mpc::lcdgui::screens::window;
using namespace mpc::lcdgui::screens;

LoadAProgramScreen::LoadAProgramScreen(mpc::Mpc& mpc, const int layerIndex) 
	: ScreenComponent(mpc, "load-a-program", layerIndex)
{
}

void LoadAProgramScreen::open()
{
	displayLoadReplaceSound();
}

void LoadAProgramScreen::turnWheel(int i)
{
	init();

	if (param == "load-replace-sound")
	{
        loadReplaceSameSound = i > 0;
		displayLoadReplaceSound();
	}
}

void LoadAProgramScreen::function(int i)
{
	init();

    auto selectedFile = mpc.screens->get<LoadScreen>("load")->getSelectedFile();
    
	switch (i)
	{
	case 2:
	{
        mpc.getSampler()->deleteAllPrograms();
        mpc.getSampler()->deleteAllSamples();

        mpc.getDisk()->readPgm2(selectedFile, mpc.getSampler()->createNewProgramAddFirstAvailableSlot().lock());
		break;
	}
	case 3:
		openScreen("load");
		break;
	case 4: {
        auto newProgram = mpc.getSampler()->createNewProgramAddFirstAvailableSlot().lock();

        mpc.getDisk()->readPgm2(selectedFile, newProgram);

        // TODO Is it correct that we only set a DRUM to the new program
        // if the active track's bus is a DRUM and not a MIDI one?
        if (mpcSoundPlayerChannel != nullptr) {
            for (int pgmIndex = 0; pgmIndex < 24; pgmIndex++) {
                if (sampler->getProgram(pgmIndex) == newProgram) {
                    mpcSoundPlayerChannel->setProgram(pgmIndex);
                    break;
                }
            }
        }

        break;
    }
	}
}

void LoadAProgramScreen::displayLoadReplaceSound()
{
	findField("load-replace-sound")->setText(std::string(loadReplaceSameSound ? "YES" : "NO(FASTER)"));
}

void LoadAProgramScreen::setLoadReplaceSameSound(bool b)
{
    loadReplaceSameSound = b;
}
