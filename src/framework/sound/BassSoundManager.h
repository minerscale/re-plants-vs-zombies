#ifndef __BASS_SOUND_MANAGER_H__
#define __BASS_SOUND_MANAGER_H__

#include "BassMusicInterface.h"
#include "BassSoundInstance.h"
#include "SoundManager.h"
#include <array>
#include <bass.h>
#include <memory>

namespace Sexy {
class BassSoundManager : public Sexy::SoundManager {
public:
    BassSoundManager(HWND theHWnd);
    static double PanDBToNorm(int dbpan);

    bool Initialized() override { return Sexy::BassMusicInterface::gBassLoaded; }

    bool LoadSound(unsigned int theSfxID, const std::string &theFilename) override;
    int LoadSound(const std::string &theFilename) override;
    void ReleaseSound(unsigned int theSfxID) override;

    void SetVolume(double theVolume) override;
    bool SetBaseVolume(unsigned int theSfxID, double theBaseVolume) override;
    bool SetBasePan(unsigned int theSfxID, int theBasePan) override;

    Sexy::SoundInstance *GetSoundInstance(unsigned int theSfxID) override;

    void ReleaseSounds() override;

    void ReleaseChannels() override {}

    double GetMasterVolume() override { return 1.0; }

    void SetMasterVolume(double) override {}

    void Flush() override {}

    //  virtual void            SetCooperativeWindow(HWND theHWnd) {}
    void StopAllSounds() override {}

    int GetFreeSoundId() override;
    int GetNumSounds() override;

private:
    std::array<std::optional<std::unique_ptr<BassSoundInstance>>, MAX_CHANNELS> mPlayingSounds;
    std::array<std::string, MAX_SOURCE_SOUNDS> mSourceFileNames;
    std::array<std::optional<HSAMPLE>, MAX_SOURCE_SOUNDS> mSourceSounds;

    bool LoadCompatibleSound(unsigned int theSfxID, const std::string &theFilename);
    bool LoadAUSound(unsigned int theSfxID, const std::string &theFilename);
    inline bool Exists(unsigned int theSfxID);
    void SetBaseVolumeAndPan(HSAMPLE theSample, std::optional<double> theBaseVolume, std::optional<int> theBasePan);
    int FindFreeChannel();
    void ReleaseFreeChannels();
};
} // namespace Sexy

#endif // __BASS_SOUND_MANAGER_H__
