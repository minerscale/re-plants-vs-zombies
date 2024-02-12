#ifndef __BASS_SOUND_INSTANCE_H__
#define __BASS_SOUND_INSTANCE_H__

#include "SoundInstance.h"
#include <bass.h>

namespace Sexy {
class BassSoundManager;

class BassSoundInstance : public Sexy::SoundInstance {
public:
    BassSoundInstance(HSAMPLE theSourceSound);
    ~BassSoundInstance() override;
    void Release() override;

    void SetBaseVolume(double /*theBaseVolume*/) override {}

    void SetBasePan(int /*theBasePan*/) override {}

    void SetVolume(double theVolume) override;
    void SetPan(int thePosition) override; //-hundredth db to +hundredth db = left to right
    void AdjustPitch(double theNumSteps) override;

    int GetSoundPosition() override;
    void SetSoundPosition(int thePosition) override;

    bool Play(bool looping, bool autoRelease) override;
    void Stop() override;
    bool IsPlaying() override;
    bool IsReleased() override;
    double GetVolume() override;

private:
    void RehupVolume() const;
    void RehupPan();

    HSAMPLE mSample;
    HCHANNEL mChannel;

    bool mAutoRelease = false;
    bool mHasPlayed = false;
    bool mReleased = false;

    int mPan = 0;
    double mVolume = 1.0;

    DWORD mDefaultFrequency = 44100;
};
} // namespace Sexy

#endif // __BASS_SOUND_INSTANCE_H__
