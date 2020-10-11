#include "NameScreen.hpp"

#include <lcdgui/screens/LoadScreen.hpp>
#include <lcdgui/screens/SongScreen.hpp>
#include <lcdgui/screens/window/DirectoryScreen.hpp>
#include <lcdgui/screens/window/MidiOutputScreen.hpp>
#include <lcdgui/screens/window/MidiOutputScreen.hpp>
#include <lcdgui/screens/window/EditSoundScreen.hpp>
#include <lcdgui/screens/window/AutoChromaticAssignmentScreen.hpp>
#include <lcdgui/screens/window/VmpcDirectToDiskRecorderScreen.hpp>
#include <lcdgui/screens/dialog/CopySoundScreen.hpp>
#include <lcdgui/screens/dialog/ResampleScreen.hpp>
#include <lcdgui/screens/dialog/StereoToMonoScreen.hpp>
#include <lcdgui/screens/dialog/MonoToStereoScreen.hpp>
#include <lcdgui/screens/dialog/CreateNewProgramScreen.hpp>
#include <lcdgui/screens/dialog2/PopupScreen.hpp>

#include <lcdgui/Underline.hpp>

#include <Util.hpp>

#include <disk/AbstractDisk.hpp>
#include <disk/MpcFile.hpp>
#include <disk/ApsSaver.hpp>

#include <sequencer/Track.hpp>
#include <sequencer/Song.hpp>

using namespace mpc::lcdgui;
using namespace mpc::lcdgui::screens;
using namespace mpc::lcdgui::screens::window;
using namespace mpc::lcdgui::screens::dialog;
using namespace mpc::lcdgui::screens::dialog2;
using namespace std;
using namespace moduru::lang;

NameScreen::NameScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "name", layerIndex)
{
	addChild(make_shared<Underline>());
}

weak_ptr<Underline> NameScreen::findUnderline()
{
	return dynamic_pointer_cast<Underline>(findChild("underline").lock());
}

void NameScreen::open()
{
	for (int i = 0; i < 16; i++)
		findUnderline().lock()->setState(i, false);

	displayName();
}

void NameScreen::close()
{
	ls.lock()->setFocus("0");
}

void NameScreen::left()
{
	init();

	if (stoi(param) == 0)
		return;

	ScreenComponent::left();
	
	if (editing)
	{
		auto field = findField(ls.lock()->getFocus()).lock();
		field->setInverted(false);
		drawUnderline();
	}
}

void NameScreen::right()
{
	init();

	if (stoi(param) == nameLimit - 1)
		return;
	
	ScreenComponent::right();
	
	if (editing)
	{
		auto field = findField(ls.lock()->getFocus()).lock();
		field->setInverted(false);
		drawUnderline();
	}
}

void NameScreen::turnWheel(int j)
{
	init();

	if (editing)
	{
		for (int i = 0; i < 16; i++)
		{
			if (param.compare(to_string(i)) == 0)
			{
				changeNameCharacter(i, j > 0);
				drawUnderline();
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < 16; i++)
		{
			if (param.compare(to_string(i)) == 0)
			{
				changeNameCharacter(i, j > 0);
				editing = true;
				initEditColors();
				drawUnderline();
				break;
			}
		}
	}
}

void NameScreen::function(int i)
{
    init();

    switch (i)
	{
    case 3:
	{
		name = originalName;

		vector<string> screens{ "save-a-program", "save-a-sequence", "save-aps-file", "save-all-file", "save-a-sound" };

		if (find(begin(screens), end(screens), parameterName) != end(screens))
			openScreen(parameterName);
		else
			openScreen(ls.lock()->getPreviousScreenName());

		resetNameScreen();
		parameterName = "";
		break;
	}
	case 4:
		saveName();
		break;
    }

}

void NameScreen::pressEnter()
{
	saveName();
}

void NameScreen::saveName()
{	
	auto prevScreen = ls.lock()->getPreviousScreenName();

	if (parameterName.compare("output-folder") == 0)
	{
		auto directToDiskRecorderScreen = dynamic_pointer_cast<VmpcDirectToDiskRecorderScreen>(mpc.screens->getScreenComponent("vmpc-direct-to-disk-recorder"));
		directToDiskRecorderScreen->outputFolder = getName();
		openScreen("vmpc-direct-to-disk-recorder");
		resetNameScreen();
	}
	else if (parameterName.compare("save-all-file") == 0)
	{
		openScreen("save-all-file");
		resetNameScreen();
		parameterName = "";
		return;
	}
	else if (parameterName.compare("save-a-sound") == 0)
	{
		openScreen("save-a-sound");
		resetNameScreen();
		parameterName = "";
		return;
	}
	else if (parameterName.compare("save-a-program") == 0)
	{
		openScreen("save-a-program");
		resetNameScreen();
		parameterName = "";
		return;
	}
	else if (parameterName.compare("save-aps-file") == 0)
	{
		string apsName = getName();
		apsName.append(".APS");
		mpc::disk::ApsSaver apsSaver(mpc, mpc::Util::getFileName(apsName));
		resetNameScreen();
		parameterName = "";
		return;
	}
	else if (parameterName.compare("save-a-sequence") == 0)
	{
		openScreen("save-a-sequence");
		resetNameScreen();
		parameterName = "";
		return;
	}
	else if (parameterName.find("default") != string::npos)
	{
		if (prevScreen.compare("track") == 0)
		{
			sequencer.lock()->setDefaultTrackName(getName(), sequencer.lock()->getActiveTrackIndex());
			openScreen("sequencer");
			resetNameScreen();
			return;
		}
		else if (prevScreen.compare("sequence") == 0)
		{
			sequencer.lock()->setDefaultSequenceName(getName());
			openScreen(prevScreen);
			resetNameScreen();
			return;
		}
		else if (prevScreen.compare("song-window") == 0)
		{
			auto songScreen = mpc.screens->get<SongScreen>("song");
			songScreen->defaultSongName = getName();
			openScreen(prevScreen);
			resetNameScreen();
			return;
		}
	}
	else if (parameterName.compare("programname") == 0)
	{
		program.lock()->setName(getName());
		openScreen("program");
		resetNameScreen();
		return;
	}
	else if (parameterName.compare("create-new-program") == 0)
	{
		auto createNewProgramScreen = dynamic_pointer_cast<CreateNewProgramScreen>(mpc.screens->getScreenComponent("create-new-program"));
		createNewProgramScreen->newName = getName();
		openScreen("program");
		resetNameScreen();
		return;
	}
	else if (parameterName.compare("autochrom") == 0)
	{
		auto autoChromaticAssignmentScreen = dynamic_pointer_cast<AutoChromaticAssignmentScreen>(mpc.screens->getScreenComponent("auto-chromatic-assignment"));
		autoChromaticAssignmentScreen->newName = getName();
		openScreen("auto-chromatic-assignment");
		resetNameScreen();
		ls.lock()->setPreviousScreenName(mpc.getPreviousSamplerScreenName());
		return;
	}
	else if (parameterName.compare("rename") == 0)
	{
		auto directoryScreen = dynamic_pointer_cast<DirectoryScreen>(mpc.screens->getScreenComponent("directory"));
		auto file = directoryScreen->getSelectedFile();
		auto ext = mpc::Util::splitName(file->getName())[1];
		
		if (ext.length() > 0)
			ext = "." + ext;

		auto disk = mpc.getDisk().lock();

		auto newName = StrUtil::trim(StrUtil::toUpper(getName())) + ext;
		auto success = file->setName(newName);

		if (!success)
		{
			openScreen("popup");
			auto popupScreen = mpc.screens->get<PopupScreen>("popup");
			popupScreen->setText("File name exists !!");
			ls.lock()->setPreviousScreenName("directory");
			return;
		}
		else
		{
			disk->flush();

			if (file->isDirectory() && directoryScreen->getXPos() == 0)
			{
				disk->moveBack();
				disk->initFiles();
				disk->moveForward(newName);
				disk->initFiles();

				auto parentFileNames = disk->getParentFileNames();
				auto it = find(begin(parentFileNames), end(parentFileNames), newName);

				auto index = distance(begin(parentFileNames), it);

				if (index > 4)
				{
					directoryScreen->setYOffset0(index - 5);
					directoryScreen->setYPos0(4);
				}
				else
				{
					directoryScreen->setYOffset0(0);
					directoryScreen->setYPos0(index);
				}
			}

			disk->initFiles();
			openScreen("directory");
			resetNameScreen();
			return;
		}
	}
	else if (parameterName.compare("newfolder") == 0)
	{
		auto disk = mpc.getDisk().lock();
		bool success = disk->newFolder(StrUtil::toUpper(getName()));

		if (success)
		{
			disk->flush();
			disk->initFiles();
			auto counter = 0;

			for (int i = 0; i < disk->getFileNames().size(); i++)
			{
				if (disk->getFileName(i).compare(StrUtil::toUpper(getName())) == 0)
				{
					auto loadScreen = dynamic_pointer_cast<LoadScreen>(mpc.screens->getScreenComponent("load"));
					loadScreen->setFileLoad(counter);

					auto directoryScreen = dynamic_pointer_cast<DirectoryScreen>(mpc.screens->getScreenComponent("directory"));

					if (counter > 4)
						directoryScreen->yOffset1 = counter - 4;
					else
						directoryScreen->yOffset1 = 0;

					break;
				}
				counter++;
			}

			openScreen("directory");
			resetNameScreen();
			ls.lock()->setPreviousScreenName("load");
		}

		if (!success)
		{
			openScreen("popup");
			auto popupScreen = mpc.screens->get<PopupScreen>("popup");
			popupScreen->setText("Folder name exists !!");
		}
	}

	if (prevScreen.compare("save-aps-file") == 0)
	{
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("keep-or-retry") == 0)
	{
		dynamic_pointer_cast<mpc::sampler::Sound>(sampler.lock()->getPreviewSound().lock())->setName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("track") == 0)
	{
		track.lock()->setName(getName());
		openScreen("sequencer");
		resetNameScreen();
	}
	else if (prevScreen.compare("save-a-sequence") == 0)
	{
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("sequence") == 0)
	{
		sequencer.lock()->getActiveSequence().lock()->setName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("song-window") == 0)
	{
		auto songScreen = mpc.screens->get<SongScreen>("song");
		sequencer.lock()->getSong(songScreen->activeSongIndex).lock()->setName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("midi-output") == 0)
	{
		auto midiOutputScreen = dynamic_pointer_cast<MidiOutputScreen>(mpc.screens->getScreenComponent("midi-output"));
		sequencer.lock()->getActiveSequence().lock()->setDeviceName(midiOutputScreen->getDeviceNumber() + 1, getName().substr(0, 8));
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("edit-sound") == 0)
	{
		auto editSoundScreen = dynamic_pointer_cast<EditSoundScreen>(mpc.screens->getScreenComponent("edit-sound"));
		editSoundScreen->setNewName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("sound") == 0)
	{
		if (sampler.lock()->isSoundNameOccupied(getName()))
			return;

		sampler.lock()->getSound().lock()->setName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("resample") == 0)
	{
		if (sampler.lock()->isSoundNameOccupied(getName()))
			return;

		auto resampleScreen = dynamic_pointer_cast<ResampleScreen>(mpc.screens->getScreenComponent("resample"));
		resampleScreen->setNewName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("stereo-to-mono") == 0)
	{
		auto stereoToMonoScreen = mpc.screens->get<StereoToMonoScreen>("stereo-to-mono");

		if (parameterName.compare("newlname") == 0)
			stereoToMonoScreen->setNewLName(getName());

		else if (parameterName.compare("newrname") == 0)
			stereoToMonoScreen->setNewRName(getName());

		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("mono-to-stereo") == 0)
	{
		auto monoToStereoScreen = mpc.screens->get<MonoToStereoScreen>("mono-to-stereo");

		monoToStereoScreen->newStName = getName();

		openScreen(prevScreen);
		resetNameScreen();
	}
	else if (prevScreen.compare("copy-sound") == 0)
	{
		auto copySoundScreen = dynamic_pointer_cast<CopySoundScreen>(mpc.screens->getScreenComponent("copy-sound"));
		copySoundScreen->setNewName(getName());
		openScreen(prevScreen);
		resetNameScreen();
	}
}

void NameScreen::drawUnderline()
{
	if (editing)
	{
		string focus = ls.lock()->getFocus();
	
		if (focus.length() != 1 && focus.length() != 2)
			return;
		
		auto u = findUnderline().lock();
		
		for (int i = 0; i < 16; i++)
			u->setState(i, i == stoi(focus));

		bringToFront(u.get());
	}
}

void NameScreen::initEditColors()
{
    for (int i = 0; i < 16; i++)
	{
		auto field = findField(to_string(i)).lock();
		field->setInverted(false);
    }

	init();
	findField(param).lock()->setInverted(false);
}

void NameScreen::resetNameScreen()
{
	editing = false;
	ls.lock()->setLastFocus("name", "0");
}

void NameScreen::setName(string name)
{
	this->name = StrUtil::padRight(name, " ", 16);
	nameLimit = 16;
	originalName = name;
}

void NameScreen::setNameLimit(int i)
{
	name = name.substr(0, i);
	nameLimit = i;
}

void NameScreen::setName(string str, int i)
{
	name[i] = str[0];
}

string NameScreen::getName()
{
	string s = name;

	while (!s.empty() && isspace(s.back()))
		s.pop_back();

	for (int i = 0; i < s.length(); i++)
		if (s[i] == ' ') s[i] = '_';

	return StrUtil::padRight(s, " ", nameLimit);
}

void NameScreen::changeNameCharacter(int i, bool up)
{
	char schar = name[i];
	string s{ schar };
	auto stringCounter = 0;
	
	for (auto str : mpc::Mpc::akaiAscii)
	{
		if (str.compare(s) == 0)
			break;

		stringCounter++;
	}

	if (stringCounter == 0 && !up)
		return;

	if (stringCounter == 75 && up)
		return;

	auto change = -1;
	
	if (up)
		change = 1;

	if (stringCounter > 75)
		s = " ";
	else
		s = mpc::Mpc::akaiAscii[stringCounter + change];
	
	name = name.substr(0, i).append(s).append(name.substr(i + 1, name.length()));

	init();
	findFocus().lock()->setText(getName().substr(stoi(param), 1));
}

void NameScreen::displayName()
{
	if (nameLimit == 0)
		return;

	findField("0").lock()->setText(getName().substr(0, 1));
	findField("1").lock()->setText(getName().substr(1, 1));
	findField("2").lock()->setText(getName().substr(2, 1));
	findField("3").lock()->setText(getName().substr(3, 1));
	findField("4").lock()->setText(getName().substr(4, 1));
	findField("5").lock()->setText(getName().substr(5, 1));
	findField("6").lock()->setText(getName().substr(6, 1));
	findField("7").lock()->setText(getName().substr(7, 1));

	if (nameLimit > 8)
	{
		findField("8").lock()->Hide(false);
		findField("9").lock()->Hide(false);
		findField("10").lock()->Hide(false);
		findField("11").lock()->Hide(false);
		findField("12").lock()->Hide(false);
		findField("13").lock()->Hide(false);
		findField("14").lock()->Hide(false);
		findField("15").lock()->Hide(false);
		findField("8").lock()->setText(getName().substr(8, 1));
		findField("9").lock()->setText(getName().substr(9, 1));
		findField("10").lock()->setText(getName().substr(10, 1));
		findField("11").lock()->setText(getName().substr(11, 1));
		findField("12").lock()->setText(getName().substr(12, 1));
		findField("13").lock()->setText(getName().substr(13, 1));
		findField("14").lock()->setText(getName().substr(14, 1));
		findField("15").lock()->setText(getName().substr(15, 1));
	}
	else
	{
		findField("8").lock()->Hide(true);
		findField("9").lock()->Hide(true);
		findField("10").lock()->Hide(true);
		findField("11").lock()->Hide(true);
		findField("12").lock()->Hide(true);
		findField("13").lock()->Hide(true);
		findField("14").lock()->Hide(true);
		findField("15").lock()->Hide(true);
	}
}