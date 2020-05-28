#include <controls/mixer/window/ChannelSettingsControls.hpp>

#include <sampler/Program.hpp>
#include <mpc/MpcStereoMixerChannel.hpp>
#include <mpc/MpcIndivFxMixerChannel.hpp>

using namespace mpc::controls::mixer::window;
using namespace std;

void ChannelSettingsControls::turnWheel(int i)
{
    init();
	auto smc = stereoMixerChannel.lock();
	auto ifmc = indivFxMixerChannel.lock();
	if (param.compare("note") == 0)
	{
		int note = mpc.getNote() + i;
	
		if (note >= 35 && note <= 98)
		{
			int pad = program.lock()->getPadIndexFromNote(note);
			mpc.setPadAndNote(note, pad);
		}
	}
	else if (param.compare("stereovolume") == 0) {
		smc->setLevel(smc->getLevel() + i);
	}
	else if (param.compare("individualvolume") == 0) {
		ifmc->setVolumeIndividualOut(ifmc->getVolumeIndividualOut() + i);
	}
	else if (param.compare("fxsendlevel") == 0) {
		ifmc->setFxSendLevel(ifmc->getFxSendLevel() + i);
	}
	else if (param.compare("panning") == 0) {
		smc->setPanning(smc->getPanning() + i);
	}
	else if (param.compare("output") == 0) {
		ifmc->setOutput(ifmc->getOutput() + i);
	}
	else if (param.compare("fxpath") == 0) {
		ifmc->setFxPath(ifmc->getFxPath() + i);
	}
	else if (param.compare("followstereo") == 0) {
		ifmc->setFollowStereo(true);
	}
}
