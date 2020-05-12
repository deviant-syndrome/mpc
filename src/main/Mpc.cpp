#include "Mpc.hpp"

#include <lcdgui/Component.hpp>

#include "Paths.hpp"
#include <nvram/NvRam.hpp>

#include <disk/AbstractDisk.hpp>
#include <disk/SoundLoader.hpp>

#include <ui/Uis.hpp>
#include <ui/disk/DiskGui.hpp>

#include <controls/Controls.hpp>

#include <audiomidi/AudioMidiServices.hpp>
#include <audiomidi/EventHandler.hpp>
#include <audiomidi/MpcMidiInput.hpp>

#include <sampler/Sampler.hpp>
#include <sequencer/Sequence.hpp>
#include <sequencer/Track.hpp>
#include <sequencer/Sequencer.hpp>
#include <mpc/MpcBasicSoundPlayerChannel.hpp>
#include <mpc/MpcMultiMidiSynth.hpp>
#include <mpc/MpcSoundPlayerChannel.hpp>

#include <midi/core/MidiTransport.hpp>
#include <synth/SynthChannel.hpp>

#include <hardware/Hardware.hpp>
#include <hardware/HwSlider.hpp>

#include <disk/AllLoader.hpp>
#include <disk/ApsLoader.hpp>

#include <disk/MpcFile.hpp>

#include <string>

using namespace mpc;
using namespace std;

Mpc::Mpc()
{
	moduru::Logger::l.setPath(mpc::Paths::logFilePath());

	hardware = make_shared<hardware::Hardware>();

	uis = make_shared<ui::Uis>();
	layeredScreen = make_shared<lcdgui::LayeredScreen>();
}

void Mpc::init(const int sampleRate, const int inputCount, const int outputCount)
{
	sequencer = make_shared<mpc::sequencer::Sequencer>();
	MLOG("Sequencer created");

	sampler = make_shared<mpc::sampler::Sampler>();
	MLOG("Sampler created");

	mpcMidiInputs = vector<mpc::audiomidi::MpcMidiInput*>{ new mpc::audiomidi::MpcMidiInput(0), new mpc::audiomidi::MpcMidiInput(1) };

	/*
	* AudioMidiServices requires sampler to exist.
	*/
	audioMidiServices = make_shared<mpc::audiomidi::AudioMidiServices>();
	MLOG("AudioMidiServices created");

	sequencer->init();
	MLOG("Sequencer initialized");

	sampler->init();
	MLOG("Sampler initialized");

	eventHandler = make_shared<mpc::audiomidi::EventHandler>();
	MLOG("Eeventhandler created");

	audioMidiServices->start(sampleRate, inputCount, outputCount);
	MLOG("AudioMidiServices started");

	controls = make_shared<controls::Controls>();

	diskController = make_unique<DiskController>();
	diskController->initDisks();

	hardware->getSlider().lock()->setValue(mpc::nvram::NvRam::getSlider());
	MLOG("Mpc is ready")
}

weak_ptr<ui::Uis> Mpc::getUis() {
	return uis;
}

weak_ptr<controls::Controls> Mpc::getControls() {
	return controls;
}

weak_ptr<hardware::Hardware> Mpc::getHardware() {
	return hardware;
}

weak_ptr<mpc::sequencer::Sequencer> Mpc::getSequencer()
{
    return sequencer;
}

weak_ptr<sampler::Sampler> Mpc::getSampler()
{
    return sampler;
}

ctoot::mpc::MpcSoundPlayerChannel* Mpc::getDrum(int i)
{
	auto mms = audioMidiServices->getMms().lock();
	auto channel = mms->getChannel(i).lock().get();
	return dynamic_cast< ctoot::mpc::MpcSoundPlayerChannel*>(channel);
}

vector<ctoot::mpc::MpcSoundPlayerChannel*> Mpc::getDrums()
{
	auto drums = vector<ctoot::mpc::MpcSoundPlayerChannel*>(4);
	for (int i = 0; i < 4; i++) {
		drums[i] = getDrum(i);
	}
	return drums;
}

ctoot::mpc::MpcBasicSoundPlayerChannel* Mpc::getBasicPlayer()
{
	auto mms = audioMidiServices->getMms().lock();
	auto channel = mms->getChannel(4).lock().get();
	return dynamic_cast< ctoot::mpc::MpcBasicSoundPlayerChannel*>(channel);
}

weak_ptr<audiomidi::AudioMidiServices> Mpc::getAudioMidiServices()
{
    return audioMidiServices;
}

weak_ptr<audiomidi::EventHandler> Mpc::getEventHandler() {
	return eventHandler;
}

weak_ptr<lcdgui::LayeredScreen> Mpc::getLayeredScreen() {
	return layeredScreen;
}

controls::BaseControls* Mpc::getActiveControls() {
	return controls->getControls(layeredScreen->getCurrentScreenName());
}

controls::GlobalReleaseControls* Mpc::getReleaseControls() {
	return controls->getReleaseControls();
}

weak_ptr<mpc::disk::AbstractDisk> Mpc::getDisk()
{
	return diskController->getDisk();
}

weak_ptr<mpc::disk::Stores> Mpc::getStores()
{
	return diskController->getStores();
}

vector<char> Mpc::akaiAsciiChar { ' ', '!', '#', '$', '%', '&', '\'', '(', ')', '-', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '}' };
vector<string> Mpc::akaiAscii { " ", "!", "#", "$", "%", "&", "'", "(", ")", "-", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "_", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "}" };

void Mpc::loadSound(bool replace)
{
	if (loadSoundThread.joinable()) {
		loadSoundThread.join();
	}
	auto lDisk = getDisk().lock();
	lDisk->setBusy(true);
	auto soundLoader = mpc::disk::SoundLoader(sampler->getSounds(), replace);
	soundLoader.setPreview(true);
	soundLoader.setPartOfProgram(false);
	bool hasNotBeenLoadedAlready = true;
	
	try {
		hasNotBeenLoadedAlready = soundLoader.loadSound(uis->getDiskGui()->getSelectedFile()) == -1;
	}
	catch (const invalid_argument& exception) {
		MLOG("A problem occurred when trying to load " + uis->getDiskGui()->getSelectedFile()->getName() + ": " + std::string(exception.what()));
		lDisk->setBusy(false);
		uis->getDiskGui()->removePopup();
		return;
	}
	
	if (hasNotBeenLoadedAlready) {
		loadSoundThread = thread(&Mpc::static_loadSound, soundLoader.getSize());
	}
	else {
		sampler->deleteSample(sampler->getSoundCount() - 1);
		lDisk->setBusy(false);
	}
}

void Mpc::loadProgram()
{
	programLoader.reset();
	programLoader = make_unique<mpc::disk::ProgramLoader>(uis->getDiskGui()->getSelectedFile(), uis->getDiskGui()->getLoadReplaceSound());
}

void Mpc::importLoadedProgram()
{
	auto t = sequencer->getActiveSequence().lock()->getTrack(sequencer->getActiveTrackIndex()).lock();
	if (uis->getDiskGui()->getClearProgramWhenLoading()) {
		auto pgm = getDrum(t->getBusNumber() - 1)->getProgram();
		sampler->replaceProgram(programLoader->get(), pgm);
	}
	else {
		getDrum(t->getBusNumber() - 1)->setProgram(sampler->getProgramCount() - 1);
	}
}

ctoot::mpc::MpcMultiMidiSynth* Mpc::getMms()
{
	return audioMidiServices->getMms().lock().get();
}

void Mpc::static_loadSound(int size)
{
	Mpc::instance().runLoadSoundThread(size);
}

void Mpc::runLoadSoundThread(int size) {
	int sleepTime = size / 400;
	if (sleepTime < 300) sleepTime = 300;
	this_thread::sleep_for(chrono::milliseconds((int)(sleepTime * 0.1)));
	uis->getDiskGui()->removePopup();
	layeredScreen->openScreen("loadasound");
	getDisk().lock()->setBusy(false);
}

weak_ptr<audiomidi::MpcMidiPorts> Mpc::getMidiPorts()
{
	return audioMidiServices->getMidiPorts();
}

audiomidi::MpcMidiInput* Mpc::getMpcMidiInput(int i)
{
	return mpcMidiInputs[i];
}

Mpc::~Mpc() {
	MLOG("Mpc destructor.");

	for (auto& m : mpcMidiInputs) {
		if (m != nullptr) {
			delete m;
		}
	}
    if (layeredScreen) layeredScreen.reset();
	if (audioMidiServices) audioMidiServices->destroyServices();
	MLOG("audio midi services destroyed.");
	if (loadSoundThread.joinable()) loadSoundThread.join();
}
