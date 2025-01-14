#include "SaveASoundScreen.hpp"

#include <lcdgui/screens/window/NameScreen.hpp>
#include <lcdgui/screens/dialog/FileExistsScreen.hpp>
#include <lcdgui/screens/dialog2/PopupScreen.hpp>

#include <disk/AbstractDisk.hpp>
#include <disk/MpcFile.hpp>
#include <Util.hpp>

using namespace mpc::lcdgui::screens::window;
using namespace mpc::lcdgui::screens::dialog;
using namespace mpc::lcdgui::screens::dialog2;

SaveASoundScreen::SaveASoundScreen(mpc::Mpc& mpc, const int layerIndex) 
	: ScreenComponent(mpc, "save-a-sound", layerIndex)
{
}

void SaveASoundScreen::open()
{
    if (ls->getPreviousScreenName() == "save")
    {
        auto nameScreen = mpc.screens->get<NameScreen>("name");
        nameScreen->setName(sampler->getSound()->getName());
    }

	displayFile();
	displayFileType();
}

void SaveASoundScreen::turnWheel(int i)
{
	init();
	
	if (param == "file" && i > 0)
	{
		sampler->selectPreviousSound();
        displayFile();
	}
	else if (param == "file" && i < 0)
	{
		sampler->selectNextSound();
        displayFile();
	}
	else if (param == "file-type")
	{
		setFileType(fileType + i);
	}
}

void SaveASoundScreen::function(int i)
{
	init();
	
	switch (i)
	{
	case 3:
		openScreen("save");
		break;
	case 4:
	{
		auto disk = mpc.getDisk();
		auto s = sampler->getSound();
		auto ext = std::string(fileType == 0 ? ".SND" : ".WAV");
		auto fileName = mpc::Util::getFileName(mpc.screens->get<NameScreen>("name")->getNameWithoutSpaces()) + ext;

        auto saveAction = [this, disk, s, fileName] {
            disk->flush();
            disk->initFiles();

            if (fileType == 0)
            {
                disk->writeSnd(s, fileName);
            }
            else
            {
                disk->writeWav(s, fileName);
            }

            disk->flush();

            auto popupScreen = mpc.screens->get<PopupScreen>("popup");
            popupScreen->setText("Saving " + fileName);
            popupScreen->returnToScreenAfterMilliSeconds("save", 700);
            mpc.getLayeredScreen()->openScreen("popup");
        };

		if (disk->checkExists(fileName))
		{
            auto replaceAction = [saveAction, disk, fileName]{
                auto success = disk->getFile(fileName)->del();

                if (success)
                {
                    saveAction();
                }
            };

            const auto initializeNameScreen = [this]{
                auto nameScreen = mpc.screens->get<NameScreen>("name");
                auto enterAction = [this](std::string&){ openScreen(name); };
                nameScreen->initialize(nameScreen->getNameWithoutSpaces(), 16, enterAction, "save");
            };

            auto fileExistsScreen = mpc.screens->get<FileExistsScreen>("file-exists");
            fileExistsScreen->initialize(replaceAction, initializeNameScreen, "save");
            openScreen("file-exists");
            return;
		}
        saveAction();
        break;
	}
	}
}

void SaveASoundScreen::setFileType(int i)
{
	if (i < 0 || i > 1)
		return;

	fileType = i;
	displayFileType();
}

void SaveASoundScreen::displayFileType()
{
	findField("file-type")->setText(std::string(fileType == 0 ? "MPC2000" : "WAV"));
}

void SaveASoundScreen::displayFile()
{
	findField("file")->setText(mpc.screens->get<NameScreen>("name")->getNameWithoutSpaces());
}
