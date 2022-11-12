#include "FrameSeq.hpp"

#include <Mpc.hpp>

#include "audiomidi/EventHandler.hpp"
#include "audiomidi/MpcMidiOutput.hpp"

#include <hardware/Hardware.hpp>
#include <hardware/HwPad.hpp>
#include <hardware/TopPanel.hpp>
#include <controls/GlobalReleaseControls.hpp>

#include <sequencer/Song.hpp>
#include <sequencer/Step.hpp>
#include <sequencer/Track.hpp>
#include <sequencer/SeqUtil.hpp>

#include <lcdgui/screens/window/TimingCorrectScreen.hpp>
#include <lcdgui/screens/window/CountMetronomeScreen.hpp>
#include <lcdgui/screens/SyncScreen.hpp>
#include <lcdgui/screens/SongScreen.hpp>
#include <lcdgui/screens/PunchScreen.hpp>
#include <lcdgui/screens/UserScreen.hpp>
#include <lcdgui/screens/SequencerScreen.hpp>

#include "mpc/MpcSoundPlayerChannel.hpp"
#include "mpc/MpcMultiMidiSynth.hpp"
#include "midi/core/ShortMessage.hpp"

#include "Util.hpp"

using namespace mpc::lcdgui;
using namespace mpc::lcdgui::screens;
using namespace mpc::lcdgui::screens::window;
using namespace mpc::sequencer;

FrameSeq::FrameSeq(mpc::Mpc& mpc)
	: mpc(mpc),
    sequencer(mpc.getSequencer()),
    countMetronomeScreen(mpc.screens->get<CountMetronomeScreen>("count-metronome"))
{
    clockMsg = std::make_shared<ctoot::midi::core::ShortMessage>();
    clockMsg->setMessage(ctoot::midi::core::ShortMessage::TIMING_CLOCK);
}

void FrameSeq::start(float sampleRate) {
	//MLOG("frameSeq starting with sampleRate " + std::to_string(sampleRate));

	if (running)
		return;

	clock.init(sampleRate);
	clock.setTick(sequencer->getPlayStartTick());
	running = true;
}

void FrameSeq::startMetronome(int sampleRate) {
	if (running)
		return;

	metronome = true;
	start(sampleRate);
}

void FrameSeq::work(int nFrames)
{
    for (auto& e : eventsAfterNFrames)
    {
        if (!e.occupied.load()) continue;

        e.frameCounter += nFrames;

        if (e.frameCounter >= e.nFrames)
        {
            e.f();
            e.reset();
        }
    }

	if (!running)
		return;

	auto controls = mpc.getControls();
	auto songScreen = mpc.screens->get<SongScreen>("song");

	frameCounter += nFrames;

	if (frameCounter > 2048)
	{
		if (!sequencer->isCountingIn() && !metronome)
        {
            sequencer->notifyTimeDisplayRealtime();
            sequencer->notifyObservers(std::string("timesignature"));
        }

		frameCounter = 0;
	}

	auto seq = sequencer->getCurrentlyPlayingSequence();
	double tempo = sequencer->getTempo();

	if (tempo != clock.getBpm())
	{
		clock.set_bpm(tempo);
		sequencer->notify("tempo");
	}

	auto punchScreen = mpc.screens->get<PunchScreen>("punch");

	auto punch = punchScreen->on && sequencer->isRecordingOrOverdubbing();

	auto punchInTime = punchScreen->time0;
	auto punchOutTime = punchScreen->time1;
	bool punchIn = punchScreen->autoPunch == 0 || punchScreen->autoPunch == 2;
	bool punchOut = punchScreen->autoPunch == 1 || punchScreen->autoPunch == 2;

	auto sequencerScreen = mpc.screens->get<SequencerScreen>("sequencer");
	auto userScreen = mpc.screens->get<UserScreen>("user");
    auto timingCorrectScreen = mpc.screens->get<TimingCorrectScreen>("timing-correct");
    auto syncScreen = mpc.screens->get<SyncScreen>("sync");

    for (int frameIndex = 0; frameIndex < nFrames; frameIndex++)
	{
		if (clock.proc())
		{
			tickFrameOffset = frameIndex;

            triggerClickIfNeeded();

			if (punch)
			{
				if (punchIn && getTickPosition() == punchInTime) {
					sequencerScreen->setPunchRectOn(0, false);
					sequencerScreen->setPunchRectOn(1, true);
				}
				else if (punchOut && getTickPosition() == punchOutTime)
				{
					sequencerScreen->setPunchRectOn(1, false);
					sequencerScreen->setPunchRectOn(2, true);
				}
			}

			if (!metronome)
			{
				if (sequencer->isCountingIn())
				{
                    if (getTickPosition() >= sequencer->countInEndPos)
                    {
                        move(sequencer->countInStartPos);
                        sequencer->setCountingIn(false);
                        sequencer->countInStartPos = -1;
                        sequencer->countInEndPos = -1;
                    }

                    continue;
				}

				if (getTickPosition() >= seq->getLastTick() - 1 &&
                    !sequencer->isSongModeEnabled() &&
                    sequencer->getNextSq() != -1)
				{
					sequencer->playToTick(getTickPosition());
                    switchToNextSequence();
					sequencer->move(0);
					seq = sequencer->getCurrentlyPlayingSequence();
					seq->initLoop();
					move(0);
					continue;
				}
				else if (sequencer->isSongModeEnabled())
				{
					if (getTickPosition() >= seq->getLastTick() - 1)
					{
						sequencer->playToTick(seq->getLastTick() - 1);
						sequencer->incrementPlayedStepRepetitions();
						auto song = sequencer->getSong(songScreen->getActiveSongIndex());
						auto step = songScreen->getOffset() + 1;

						auto doneRepeating = sequencer->getPlayedStepRepetitions() >= song->getStep(step).lock()->getRepeats();
						auto reachedLastStep = step == song->getStepCount() - 1;

						if (doneRepeating && songScreen->isLoopEnabled() && step == song->getLastStep())
						{
							sequencer->playToTick(seq->getLastTick() - 1);
							sequencer->resetPlayedStepRepetitions();
							songScreen->setOffset(song->getFirstStep() - 1);
							auto newStep = song->getStep(songScreen->getOffset() + 1).lock();

							if (!sequencer->getSequence(newStep->getSequence())->isUsed())
                            {

                                sequencer->playToTick(seq->getLastTick() - 1);
                                sequencer->stop();
                                move(0);
                                continue;
                            }

							move(0);
						}
						else if (doneRepeating && reachedLastStep)
						{
							sequencer->playToTick(seq->getLastTick() - 1);
							sequencer->setEndOfSong(true);
							sequencer->stop();
							sequencer->move(0);
							continue;
						}
						else
						{
							sequencer->playToTick(seq->getLastTick() - 1);

							if (doneRepeating)
							{
								sequencer->resetPlayedStepRepetitions();
								songScreen->setOffset(songScreen->getOffset() + 1);

								auto newStep = song->getStep(songScreen->getOffset() + 1).lock();

								if (!sequencer->getSequence(newStep->getSequence())->isUsed())
                                {
                                    sequencer->playToTick(seq->getLastTick() - 1);
                                    sequencer->stop();
                                    move(0);
                                    continue;
                                }
							}

							move(0);
						}
					}
				}
				else if (seq->isLoopEnabled())
				{
					if (getTickPosition() >= seq->getLoopEnd() - 1)
					{
						if (punch && punchIn)
						{
							sequencerScreen->setPunchRectOn(0, true);
							sequencerScreen->setPunchRectOn(1, false);
						}

						if (punch && punchOut)
							sequencerScreen->setPunchRectOn(2, false);

						if (punch && punchOut && !punchIn)
							sequencerScreen->setPunchRectOn(1, true);

						sequencer->playToTick(getTickPosition());
						move(seq->getLoopStart());

						if (sequencer->isRecordingOrOverdubbing())
						{
							if (sequencer->isRecording())
								sequencer->switchRecordToOverDub();
                        }

						continue;
					}
				}
				else
				{
					if (getTickPosition() >= seq->getLastTick())
					{
						if (sequencer->isRecordingOrOverdubbing())
						{
							seq->insertBars(1, seq->getLastBarIndex());
							seq->setTimeSignature(seq->getLastBarIndex(), seq->getLastBarIndex(), userScreen->timeSig.getNumerator(), userScreen->timeSig.getDenominator());
						}
						else
						{
							sequencer->stop(seq->getLastTick());
							sequencer->move(seq->getLastTick());
						}
					}
				}
			}

            sequencer->playToTick(getTickPosition());

            if (controls && (controls->isTapPressed() || controls->isNoteRepeatLocked()))
            {
                int tcValue = sequencer->getTickValues()[timingCorrectScreen->getNoteValue()];
                int swingPercentage = timingCorrectScreen->getSwing();
                int swingOffset = (int)((swingPercentage - 50) * (4.0 * 0.01) * (tcValue * 0.5));
                auto shiftTiming = timingCorrectScreen->getAmount() * (timingCorrectScreen->isShiftTimingLater() ? 1 : -1);
                auto tickPosWithShift = getTickPosition() - shiftTiming;

                if (tcValue == 24 || tcValue == 48)
                {
                    if (tickPosWithShift % (tcValue * 2) == swingOffset + tcValue ||
                        tickPosWithShift % (tcValue * 2) == 0)
                    {
                        repeatPad(tcValue);
                    }
                }
                else if (tcValue != 1 && tickPosWithShift % tcValue == 0)
                {
                    repeatPad(tcValue);
                }
            }

            if (getTickPosition() % 4 == 0 && syncScreen->getModeOut() > 0)
            {
                clockMsg->bufferPos = frameIndex;

                if (syncScreen->getOut() == 0 || syncScreen->getOut() == 2)
                {
                    mpc.getMidiOutput()->enqueMessageOutputA(clockMsg);
                }
                if (syncScreen->getOut() == 1 || syncScreen->getOut() == 2)
                {
                    mpc.getMidiOutput()->enqueMessageOutputB(clockMsg);
                }
            }
        }
	}
}

int FrameSeq::getEventFrameOffset()
{
	return tickFrameOffset;
}

void FrameSeq::stop()
{
	if (!running)
		return;

	running = false;
	metronome = false;
	tickFrameOffset = 0;
}

bool FrameSeq::isRunning()
{
	return running;
}

int FrameSeq::getTickPosition()
{
	return clock.getTickPosition();
}

void FrameSeq::move(int newTickPos)
{
	// Ugly hack to make sure the expected clock position is shown when
    // sequencer->move(...) notifies observers.
    clock.setTick(newTickPos + 1);

    sequencer->move(newTickPos);
    clock.setTick(newTickPos);
}

void FrameSeq::repeatPad(int duration)
{
    auto track = sequencer->getActiveTrack();

    if (mpc.getLayeredScreen()->getCurrentScreenName() != "sequencer" ||
        track->getBus() == 0)
    {
        return;
    }

	auto controls = mpc.getControls();

	if (!controls)
		return;

  auto program = mpc.getSampler()->getProgram(mpc.getDrum(track->getBus() - 1)->getProgram());
  auto hardware = mpc.getHardware();
  auto fullLevel = hardware->getTopPanel()->isFullLevelEnabled();

    for (auto& p : mpc.getHardware()->getPads())
  {
    if (!p->isPressed()) continue;

    auto padIndex = p->getPadIndexWithBankWhenLastPressed();

    auto note = program->getNoteFromPad(padIndex);

    if (note != 34)
    {
        auto tick = getTickPosition();
        auto noteEvent = std::make_shared<NoteEvent>(note);
        noteEvent->setTick(tick);
        noteEvent->setNote(note);
        mpc::Util::setSliderNoteVariationParameters(mpc, noteEvent, program);
        noteEvent->setVelocity(fullLevel ? 127 : p->getPressure());
        noteEvent->setDuration(duration);

        noteEvent->getNoteOff().lock()->setTick(tick + duration);
        noteEvent->getNoteOff().lock()->setVelocity(0);
        auto eventFrame = getEventFrameOffset();

        MidiAdapter midiAdapter;

        midiAdapter.process(noteEvent->getNoteOff(), track->getBus() - 1, 0);
        auto voiceOverlap = program->getNoteParameters(note)->getVoiceOverlap();
        mpc.getMms()->mpcTransport(midiAdapter.get().lock().get(), 0, 0, 0, eventFrame, -1, -1);

        auto newVelo = static_cast<int>(noteEvent->getVelocity() * (track->getVelocityRatio() * 0.01));
        midiAdapter.process(noteEvent, track->getBus() - 1, newVelo);
        auto durationFrames = duration == -1 ? -1 : mpc::sequencer::SeqUtil::ticksToFrames(duration, clock.getBpm(), clock.getSampleRate());

        mpc.getMms()->mpcTransport(midiAdapter.get().lock().get(), 0, noteEvent->getVariationType(), noteEvent->getVariationValue(), eventFrame, -1, voiceOverlap == 2 ? durationFrames : -1);
        p->notifyObservers(newVelo);

        if (sequencer->isRecordingOrOverdubbing())
        {
            track->insertEventWhileRetainingSort(noteEvent);
        }
    }
  }
}

void FrameSeq::switchToNextSequence()
{
	sequencer->setCurrentlyPlayingSequenceIndex(sequencer->getNextSq());
	sequencer->setNextSq(-1);
}

void FrameSeq::triggerClickIfNeeded()
{
    if (!sequencer->isCountEnabled())
        return;

    if (sequencer->isRecordingOrOverdubbing() && !countMetronomeScreen->getInRec() && !sequencer->isCountingIn())
        return;

    auto isStepEditor = mpc.getLayeredScreen()->getCurrentScreenName() == "step-editor";

    if (!isStepEditor &&
        sequencer->isPlaying() &&
        !sequencer->isRecordingOrOverdubbing() &&
        !countMetronomeScreen->getInPlay() &&
        !sequencer->isCountingIn())
        return;

    auto pos = getTickPosition();
    auto bar = sequencer->getCurrentBarIndex();
    auto seq = sequencer->getActiveSequence();
    auto firstTickOfBar = seq->getFirstTickOfBar(bar);
    auto relativePos = pos - firstTickOfBar;

    if (isStepEditor && relativePos == 0)
    {
        return;
    }

    auto den = seq->getDenominator(bar);
    auto denTicks = (int)(96 * (4.0 / den));

    switch (countMetronomeScreen->getRate())
    {
        case 1:
            denTicks *= 2.0f / 3;
            break;
        case 2:
            denTicks *= 1.0f / 2;
            break;
        case 3:
            denTicks *= 1.0f / 3;
            break;
        case 4:
            denTicks *= 1.0f / 4;
            break;
        case 5:
            denTicks *= 1.0f / 6;
            break;
        case 6:
            denTicks *= 1.0f / 8;
            break;
        case 7:
            denTicks *= 1.0f / 12;
            break;
    }

    if (relativePos % denTicks == 0)
    {
        NoteEvent clickEvent(1);
        clickEvent.setVelocity(relativePos == 0 ? 127 : 64);
        mpc.getSampler()->playMetronome(&clickEvent, getEventFrameOffset());
        return;
    }
}