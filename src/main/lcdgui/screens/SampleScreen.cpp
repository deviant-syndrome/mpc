#include "SampleScreen.hpp"

#include <audiomidi/AudioMidiServices.hpp>
#include <audiomidi/SoundRecorder.hpp>

#include <cmath>

using namespace mpc::lcdgui::screens;

SampleScreen::SampleScreen(mpc::Mpc& mpc, const int layerIndex)
	: ScreenComponent(mpc, "sample", layerIndex)
{
}

void SampleScreen::open()
{
	ls->getCurrentBackground()->setName("sample");

	displayInput();
	displayThreshold();
	displayMode();
	displayTime();
	displayMonitor();
	displayPreRec();

	auto ams = mpc.getAudioMidiServices();
	ams->getSoundRecorder()->addObserver(this);
}

void SampleScreen::close()
{
	auto ams = mpc.getAudioMidiServices();
	ams->getSoundRecorder()->deleteObserver(this);

}

void SampleScreen::left()
{
	if (mpc.getAudioMidiServices()->isRecordingSound())
		return;

	ScreenComponent::left();
}

void SampleScreen::right()
{
	if (mpc.getAudioMidiServices()->isRecordingSound())
		return;

	ScreenComponent::right();
}

void SampleScreen::up()
{
	if (mpc.getAudioMidiServices()->isRecordingSound())
		return;

	ScreenComponent::up();
}

void SampleScreen::down()
{
	if (mpc.getAudioMidiServices()->isRecordingSound())
		return;

	ScreenComponent::down();
}

void SampleScreen::openWindow()
{
	openScreen("sound-memory");
}

void SampleScreen::function(int i)
{
	init();

	switch (i)
	{
	case 0:
		if (mpc.getAudioMidiServices()->isRecordingSound())
			return;

		peakL = 0.f;
		peakR = 0.f;
		break;
	case 4:
		if (mpc.getAudioMidiServices()->isRecordingSound())
		{
			mpc.getAudioMidiServices()->stopSoundRecorder(true);
			findBackground()->setName("sample");
		}
		else if (mpc.getAudioMidiServices()->getSoundRecorder()->isArmed())
		{
			mpc.getAudioMidiServices()->getSoundRecorder()->setArmed(false);
			sampler->deleteSound(sampler->getSoundCount() - 1);
			findBackground()->setName("sample");
		}
		break;
	case 5:
		if (mpc.getControls()->isF6Pressed())
			return;

		mpc.getControls()->setF6Pressed(true);

		auto ams = mpc.getAudioMidiServices();

		if (ams->isRecordingSound())
		{
			ams->stopSoundRecorder();
			return;
		}

		if (ams->getSoundRecorder()->isArmed())
		{
			ams->startRecordingSound();
			findBackground()->setName("recording");
		}
		else
		{
			auto sound = sampler->addSound();
			sound->setName(sampler->addOrIncreaseNumber("sound1"));
			auto lengthInFrames = time * (44100 * 0.1);
			ams->getSoundRecorder()->prepare(sound, lengthInFrames);
			ams->getSoundRecorder()->setArmed(true);
			findBackground()->setName("waiting-for-input-signal");
		}

		break;
	}
}

void SampleScreen::turnWheel(int i)
{
    init();
	auto ams = mpc.getAudioMidiServices();

	if (!ams->isRecordingSound())
	{
	    if (param.compare("input") == 0)
		{
            setInput(input + i);
        }
		else if (param.compare("threshold") == 0)
		{
			setThreshold(threshold + i);
		}
		else if (param.compare("mode") == 0)
		{
			setMode(mode + i);
		}
		else if (param.compare("time") == 0)
		{
			setTime(time + i);
		}
		else if (param.compare("monitor") == 0)
		{
			setMonitor(monitor + i);
			bool muteMonitor = monitor == 0;
			mpc.getAudioMidiServices()->muteMonitor(muteMonitor);
		}
		else if (param.compare("prerec") == 0)
		{
			setPreRec(preRec + i);
		}
    }
}


void SampleScreen::setInput(int i)
{
	if (i < 0 || i > 1)
		return;

	input = i;
	displayInput();
}

void SampleScreen::setThreshold(int i)
{
	if (i < -64 || i > 0)
		return;

	threshold = i;
	displayThreshold();
}

void SampleScreen::setMode(int i)
{
	if (i < 0 || i > 2)
		return;

	mode = i;
	displayMode();
}

void SampleScreen::setTime(int i)
{
	if (i < 0 || i > 3786)
		return;

	time = i;
	displayTime();
}

void SampleScreen::setMonitor(int i)
{
	if (i < 0 || i > 5)
		return;

	monitor = i;
	displayMonitor();
}

void SampleScreen::setPreRec(int i)
{
	if (i < 0 || i > 100)
		return;

	preRec = i;
	displayPreRec();
}

void SampleScreen::displayInput()
{
	findField("input")->setText(inputNames[input]);
}

void SampleScreen::displayThreshold()
{
	auto thresholdText = threshold == -64 ? u8"-\u00D9\u00DA" : std::to_string(threshold);
	findField("threshold")->setTextPadded(thresholdText);
}

void SampleScreen::displayMode()
{
	findField("mode")->setText(modeNames[mode]);
}

void SampleScreen::displayTime()
{
	auto timeText = std::to_string(time);
	timeText = timeText.substr(0, timeText.length() - 1) + "." + timeText.substr(timeText.length() - 1);
	findField("time")->setTextPadded(timeText);
}

void SampleScreen::displayMonitor()
{
	findField("monitor")->setText(monitorNames[monitor]);
}

void SampleScreen::displayPreRec()
{
	findField("prerec")->setTextPadded(preRec);
}

void SampleScreen::updateVU(const float levelL, const float levelR)
{
    std::string lString;
    std::string rString;

	int peaklValue = (int) floor(log10(peakL) * 20);
	int peakrValue = (int) floor(log10(peakR) * 20);
	
	int levell = (int) floor(log10(levelL) * 20);
	int levelr = (int) floor(log10(levelR) * 20);

	for (int i = 0; i < 34; i++)
	{
        std::string l = " ";
        std::string r = " ";
		bool normall = vuPosToDb[i] <= levell;
		bool normalr = vuPosToDb[i] <= levelr;
		
		bool thresholdHit = threshold >= vuPosToDb[i] && (i == 33 || threshold < vuPosToDb[i + 1]);
		
		bool peakl = peaklValue >= vuPosToDb[i] && (i == 33 || peaklValue < vuPosToDb[i + 1]);
		bool peakr = peakrValue >= vuPosToDb[i] && (i == 33 || peakrValue < vuPosToDb[i + 1]);

		if (thresholdHit && peakl) l = vu_peak_threshold;
		if (thresholdHit && peakr) r = vu_peak_threshold;

		if (thresholdHit && normall && !peakl) l = vu_normal_threshold;
		if (thresholdHit && normalr && !peakr) r = vu_normal_threshold;

		if (thresholdHit && !peakl && !normall) l = vu_threshold;
		if (thresholdHit && !peakr && !normalr) r = vu_threshold;

		if (normall && !peakl && !thresholdHit) l = vu_normal;
		if (normalr && !peakr && !thresholdHit) r = vu_normal;

		if (peakl && !thresholdHit) l = vu_peak;
		if (peakr && !thresholdHit) r = vu_peak;

		if (peakl && thresholdHit && levell == 33) l = vu_peak_threshold_normal;
		if (peakr && thresholdHit && levelr == 33) r = vu_peak_threshold_normal;

		lString += l;
		rString += r;
	}

	findLabel("vuleft")->setText( (mode == 0 || mode == 2) ? lString : "                                  ");
	findLabel("vuright")->setText( (mode == 1 || mode == 2) ? rString : "                                  ");
}

int SampleScreen::getMode()
{
	return mode;
}

int SampleScreen::getMonitor()
{
	return monitor;
}

void SampleScreen::update(moduru::observer::Observable* o, nonstd::any arg)
{
	if (dynamic_cast<mpc::audiomidi::SoundRecorder*>(o) != nullptr)
	{
		try
		{
			auto vuValue = nonstd::any_cast<std::pair<float, float>>(arg);
			auto left = vuValue.first;
			auto right = vuValue.second;
			if (left > peakL) peakL = left;
			if (right > peakR) peakR = right;
			updateVU(left, right);
		}
		catch (const std::exception& e) {
			// nothing to do
		}
		return;
	}
}
