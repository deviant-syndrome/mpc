#include "VelocityModulationScreen.hpp"

#include <sampler/NoteParameters.hpp>

using namespace mpc::lcdgui::screens::window;
using namespace mpc::controls;
using namespace moduru::lang;

VelocityModulationScreen::VelocityModulationScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "velocity-modulation", layerIndex)
{
}

void VelocityModulationScreen::open()
{
	init();
	displayNote();
	displayVeloAttack();
	displayVeloStart();
	displayVeloLevel();
	displayVelo();

	mpc.addObserver(this); // Subscribe to "note" messages
}

void VelocityModulationScreen::close()
{
	mpc.deleteObserver(this);
}

void VelocityModulationScreen::turnWheel(int i)
{
	init();

	auto lastNp = sampler->getLastNp(program.get());

	if (param == "veloattack")
	{
		lastNp->setVelocityToAttack(lastNp->getVelocityToAttack() + i);
		displayVeloAttack();
	}
	else if (param == "velostart")
	{
		lastNp->setVelocityToStart(lastNp->getVelocityToStart() + i);
		displayVeloStart();
	}
	else if (param == "velolevel")
	{
		lastNp->setVeloToLevel(lastNp->getVeloToLevel() + i);
		displayVeloLevel();
	}
	else if (param == "note")
	{
		mpc.setNote(mpc.getNote() + i);
		displayNote();
	}
}

void VelocityModulationScreen::displayNote()
{
	auto noteParameters = sampler->getLastNp(program.get());
	auto soundIndex = noteParameters->getSoundIndex();
	auto padIndex = program->getPadIndexFromNote(noteParameters->getNumber());
	auto padName = sampler->getPadName(padIndex);
	auto sampleName = soundIndex != -1 ? sampler->getSoundName(soundIndex) : "OFF";
    std::string stereo = soundIndex != -1 && !sampler->getSound(soundIndex)->isMono() ? "(ST)" : "";
	findField("note")->setText(std::to_string(noteParameters->getNumber()) + "/" + padName + "-" + StrUtil::padRight(sampleName, " ", 16) + stereo);
}

void VelocityModulationScreen::displayVelo()
{
	findField("velo")->setText("127"); // Unimplemented.
}

void VelocityModulationScreen::displayVeloAttack()
{
	findField("veloattack")->setTextPadded(sampler->getLastNp(program.get())->getVelocityToAttack(), " ");
}

void VelocityModulationScreen::displayVeloStart()
{
	findField("velostart")->setTextPadded(sampler->getLastNp(program.get())->getVelocityToStart(), " ");
}

void VelocityModulationScreen::displayVeloLevel()
{
	findField("velolevel")->setTextPadded(sampler->getLastNp(program.get())->getVeloToLevel(), " ");
}

void VelocityModulationScreen::update(moduru::observer::Observable* observable, nonstd::any message)
{
	auto msg = nonstd::any_cast<std::string>(message);

	if (msg == "note")
	{
		displayNote();
		displayVeloAttack();
		displayVeloStart();
		displayVeloLevel();
	}
}
