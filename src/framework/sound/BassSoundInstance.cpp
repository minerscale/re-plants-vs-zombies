#include "BassSoundInstance.h"
#include "BassSoundManager.h"
#include <bass.h>

namespace Sexy {
BassSoundInstance::BassSoundInstance(HSAMPLE theSourceSound) : mSample(theSourceSound) {
    mChannel = BASS_SampleGetChannel(mSample, 0);
}

BassSoundInstance::~BassSoundInstance() { BASS_ChannelStop(mChannel); }

void BassSoundInstance::RehupVolume() const { BASS_ChannelSetAttribute(mChannel, BASS_ATTRIB_VOL, mVolume); }

void BassSoundInstance::RehupPan() const {
    BASS_ChannelSetAttribute(mChannel, BASS_ATTRIB_PAN, BassSoundManager::PanDBToNorm(mPan));
}

void BassSoundInstance::Release() {
    Stop();
    mReleased = true;
}

void BassSoundInstance::SetVolume(double theVolume) // 0 = max
{
    mVolume = theVolume;
    RehupVolume();
}

void BassSoundInstance::SetPan(int thePosition) //-db to =db = left to right
{
    mPan = thePosition;
    RehupPan();
}

bool BassSoundInstance::Play(bool looping, bool autoRelease) {
    Stop();

    mHasPlayed = true;
    mAutoRelease = autoRelease;

    BASS_ChannelFlags(mChannel, looping ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);

    if (BASS_ErrorGetCode() != BASS_OK) return false;

    return BASS_ChannelStart(mChannel);
}

void BassSoundInstance::Stop() {
    BASS_ChannelPause(mChannel);
    mAutoRelease = false;
}

const double ONE_SEMITONE = pow(2.0, 1.0 / 12.0);

void BassSoundInstance::AdjustPitch(double theNumSteps) {
    const double aFrequencyMult = pow(ONE_SEMITONE, theNumSteps);
    const double aNewFrequency = mDefaultFrequency * aFrequencyMult;

    /*
    if (aNewFrequency < DSBFREQUENCY_MIN)
        aNewFrequency = DSBFREQUENCY_MIN;
    if (aNewFrequency > DSBFREQUENCY_MAX)
        aNewFrequency = DSBFREQUENCY_MAX;*/

    BASS_ChannelSetAttribute(mChannel, BASS_ATTRIB_FREQ, aNewFrequency);
}

int BassSoundInstance::GetSoundPosition() { return BASS_ChannelGetPosition(mChannel, BASS_POS_BYTE); }

void BassSoundInstance::SetSoundPosition(int thePosition) {
    BASS_ChannelSetPosition(mChannel, BASS_POS_BYTE, thePosition);
}

bool BassSoundInstance::IsPlaying() { return (BASS_ChannelIsActive(mChannel) == BASS_ACTIVE_PLAYING); }

bool BassSoundInstance::IsReleased() {
    if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying())) Release();

    return mReleased;
}

double BassSoundInstance::GetVolume() { return mVolume; }
} // namespace Sexy
