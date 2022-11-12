#include "TrackScreen.hpp"

#include <sequencer/Track.hpp>

#include <lcdgui/screens/window/NameScreen.hpp>

using namespace mpc::lcdgui::screens::window;

TrackScreen::TrackScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "track", layerIndex)
{
}

void TrackScreen::open()
{
	ls.lock()->setPreviousScreenName("sequencer");

	init();
	auto activeTrackIndex = sequencer->getActiveTrackIndex();
	auto defaultTrackName = sequencer->getDefaultTrackName(activeTrackIndex);

	findField("tracknamefirstletter")->setText(track.lock()->getName().substr(0, 1));
	findLabel("tracknamerest")->setText(track.lock()->getName().substr(1));

	findField("defaultnamefirstletter")->setText(defaultTrackName.substr(0, 1));
	findLabel("defaultnamerest")->setText(defaultTrackName.substr(1));
}

void TrackScreen::function(int i)
{
	ScreenComponent::function(i);
	switch (i)
	{
	case 1:
		openScreen("delete-track");
		break;
	case 4:
		openScreen("copy-track");
		break;
	}
}

void TrackScreen::turnWheel(int i)
{
	init();
	auto nameScreen = mpc.screens->get<NameScreen>("name");
    std::function<void(std::string&)> renamer;
    
	if (param.find("default") != std::string::npos)
	{
        nameScreen->setName(sequencer->getDefaultTrackName(sequencer->getActiveTrackIndex()));
        
        renamer = [&](std::string& newName) {
            sequencer->setDefaultTrackName(newName, sequencer->getActiveTrackIndex());
        };
	}
	else
	{
        const auto _track = track.lock();
		if (!_track->isUsed())
			_track->setUsed(true);

		nameScreen->setName(_track->getName());
        
        renamer = [_track](std::string& newName) {
            _track->setName(newName);
        };
	}

    nameScreen->setRenamerAndScreenToReturnTo(renamer, "sequencer");
    openScreen("name");
}
