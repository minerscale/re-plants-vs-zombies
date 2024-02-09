#ifndef __DUMMY_SOUND_INSTANCE_H__
#define __DUMMY_SOUND_INSTANCE_H__

#include "sound/SoundInstance.h"

class DummySoundInstance : public Sexy::SoundInstance {
public:
    void Release() override {}

    void SetBaseVolume(double) override {}

    void SetBasePan(int) override {}

    void AdjustPitch(double) override {}

    void SetVolume(double) override {}

    void SetPan(int) override {} //-hundredth db to +hundredth db = left to right

    int GetSoundPosition() override { return 0; }

    void SetSoundPosition(int /*thePosition*/) override {}

    bool Play(bool, bool) override { return true; }

    void Stop() override {}

    bool IsPlaying() override { return false; }
    bool IsReleased() override { return true; }
    double GetVolume() override { return 0; }
};

#endif // __DUMMY_SOUND_INSTANCE_H__
