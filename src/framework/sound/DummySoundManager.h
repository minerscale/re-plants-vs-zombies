#ifndef __DUMMY_SOUND_MANAGER_H__
#define __DUMMY_SOUND_MANAGER_H__

#include "DummySoundInstance.h"
#include "SoundManager.h"

class DummySoundManager : public Sexy::SoundManager {
public:
    bool Initialized() override { return true; }

    bool LoadSound(unsigned int, const std::string &) override { return true; }
    int LoadSound(const std::string &) override { return 0; }

    void ReleaseSound(unsigned int) override {}

    void SetVolume(double) override {}

    bool SetBaseVolume(unsigned int, double) override { return 0; }
    bool SetBasePan(unsigned int, int) override { return 0; }

    Sexy::SoundInstance *GetSoundInstance(unsigned int) override { return &dummy; }

    void ReleaseSounds() override {}

    void ReleaseChannels() override {}

    double GetMasterVolume() override { return 0; }

    void SetMasterVolume(double) override {}

    void Flush() override {}

    //	virtual void			SetCooperativeWindow(HWND theHWnd) {}
    void StopAllSounds() override {}

    int GetFreeSoundId() override { return 0; }
    int GetNumSounds() override { return 0; }

private:
    DummySoundInstance dummy;
};

#endif // __DUMMY_SOUND_MANAGER_H__
