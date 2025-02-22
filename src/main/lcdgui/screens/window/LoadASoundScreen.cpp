#include "LoadASoundScreen.hpp"

#include <sampler/Pad.hpp>

#include <sequencer/Track.hpp>

#include <disk/MpcFile.hpp>

#include <lcdgui/screens/LoadScreen.hpp>
#include <lcdgui/screens/window/NameScreen.hpp>
#include <lcdgui/screens/dialog/FileExistsScreen.hpp>

#include <mpc/MpcStereoMixerChannel.hpp>

using namespace mpc::lcdgui::screens::window;
using namespace mpc::lcdgui::screens::dialog;
using namespace mpc::lcdgui::screens;

LoadASoundScreen::LoadASoundScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "load-a-sound", layerIndex)
{
}

void LoadASoundScreen::mainScreen()
{
    sampler->deleteSound(sampler->getPreviewSound());
    ScreenComponent::mainScreen();
}

void LoadASoundScreen::open()
{
	init();
	auto loadScreen = mpc.screens->get<LoadScreen>("load");
	findLabel("filename")->setText("File:" + loadScreen->getSelectedFile()->getNameWithoutExtension());
    assignToNote = mpc.getNote();
	displayAssignToNote();
	mpc.addObserver(this); // Subscribe to "note" messages
}

void LoadASoundScreen::close()
{
	mpc.deleteObserver(this);
}

void LoadASoundScreen::turnWheel(int i)
{
	init();

	if (param == "assign-to-note")
	{
		auto newAssignToNote = assignToNote + i;

        if (newAssignToNote < 34)
        {
            newAssignToNote = 34;
        }
        else if (newAssignToNote > 98)
        {
            newAssignToNote = 98;
        }

        if (newAssignToNote == 34)
        {
            mpc.setNote(35);
            assignToNote = newAssignToNote;
            displayAssignToNote();
        }
        else
        {
            mpc.setNote(newAssignToNote);
        }
    }
}

void LoadASoundScreen::function(int i)
{
	init();
	auto controls = mpc.getControls();
	
	switch (i)
	{
	case 2:
	{
		if (controls->isF3Pressed())
			return;

		controls->setF3Pressed(true);
		auto s = sampler->getPreviewSound();
		auto start = s->getStart();
		auto end = s->getLastFrameIndex();
		auto loopTo = -1;

		if (s->isLoopEnabled())
		{
			loopTo = s->getLoopTo();
		}

        sampler->playPreviewSample(start, end, loopTo);
		break;
	}
	case 3:
		sampler->finishBasicVoice(); // Here we make sure the sound is not being played, so it can be removed from memory.
		sampler->deleteSound(sampler->getPreviewSound());
		openScreen("load");
		break;
	case 4:
		keepSound();
		break;
	}
}

void LoadASoundScreen::keepSound()
{
    auto previewSound = sampler->getPreviewSound();
    auto candidateSoundName = previewSound->getName();

    int existingSoundIndex = -1;

    for (int i = 0; i < sampler->getSoundCount(); i++)
    {
        auto s = sampler->getSound(i);
        if (s == previewSound) continue;

        if (moduru::lang::StrUtil::eqIgnoreCase(s->getName(), candidateSoundName))
        {
            existingSoundIndex = i;
            break;
        }
    }

    auto actionAfterLoadingSound = [this](bool newSoundIsMono){
        if (assignToNote != 34)
        {
            auto sequence = sequencer->getActiveSequence();
            auto track = sequence->getTrack(sequencer->getActiveTrackIndex());

            auto bus = track->getBus();
            auto programNumber = sampler->getDrumBusProgramIndex(bus);
            auto program = sampler->getProgram(programNumber);
            auto noteParameters = sampler->getLastNp(program.get());
            noteParameters->setSoundIndex(sampler->getSoundCount() - 1);
        }
    };

    if (existingSoundIndex >= 0)
    {
        auto replaceAction = [this, existingSoundIndex, actionAfterLoadingSound]{
            auto previewSound = sampler->getPreviewSound();
            const auto isMono = previewSound->isMono();
            sampler->replaceSound(existingSoundIndex, previewSound);
            actionAfterLoadingSound(isMono);
            openScreen("load");
        };

        const auto initializeNameScreen = [this, &actionAfterLoadingSound, &previewSound]{
            auto nameScreen = mpc.screens->get<NameScreen>("name");

            auto enterAction = [this, &actionAfterLoadingSound, &previewSound](std::string& nameScreenName){
                if (sampler->checkExists(nameScreenName))
                {
                    return;
                }

                previewSound->setName(nameScreenName);
                actionAfterLoadingSound(previewSound->isMono());
                openScreen("load");
            };

            auto loadScreen = mpc.screens->get<LoadScreen>("load");
            auto mainScreenAction = [&](){ sampler->deleteSound(sampler->getPreviewSound()); };
            nameScreen->initialize(loadScreen->getSelectedFile()->getNameWithoutExtension(), 16, enterAction, "load", mainScreenAction);
        };

        auto fileExistsScreen = mpc.screens->get<FileExistsScreen>("file-exists");
        fileExistsScreen->initialize(replaceAction, initializeNameScreen, "load");
        openScreen("file-exists");
        return;
    }

    actionAfterLoadingSound(previewSound->isMono());
    openScreen("load");
}

void LoadASoundScreen::displayAssignToNote()
{
	init();
	auto padIndex = program->getPadIndexFromNote(assignToNote);
	auto padName = sampler->getPadName(padIndex);
	auto noteName = std::string(assignToNote == 34 ? "--" : std::to_string(assignToNote));
	findField("assign-to-note")->setText(noteName + "/" + padName);
}

void LoadASoundScreen::update(moduru::observer::Observable* observable, nonstd::any message)
{
	auto msg = nonstd::any_cast<std::string>(message);
	if (msg == "note")
	{
        assignToNote = mpc.getNote();
		displayAssignToNote();
	}
}
