#ifndef __BASSMUSICINTERFACE_H__
#define __BASSMUSICINTERFACE_H__

#include "MusicInterface.h"
#include "bass.h"

namespace Sexy {
#ifndef _WIN32
#define HWND void *
#endif

class SexyAppBase;

class BassMusicInfo {
public:
    HMUSIC mHMusic;
    HSTREAM mHStream;
    double mVolume;
    double mVolumeAdd;
    double mVolumeCap;
    bool mStopOnFade;

public:
    BassMusicInfo();

    DWORD GetHandle() { return mHMusic ? mHMusic : mHStream; }
};

using BassMusicMap = std::map<int, BassMusicInfo>;

class BassMusicInterface : public MusicInterface {
public:
    static bool gBassLoaded;
    static void InitBass(HWND theHWnd);

    BassMusicMap mMusicMap;
    int mMaxMusicVolume;
    int mMusicLoadFlags;

public:
    BassMusicInterface(HWND theHWnd);
    ~BassMusicInterface() override;

    bool LoadMusic(int theSongId, const std::string &theFileName) override;
    void PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false) override;
    void StopMusic(int theSongId) override;
    void StopAllMusic() override;
    void UnloadMusic(int theSongId) override;
    void UnloadAllMusic() override;
    void PauseAllMusic() override;
    void ResumeAllMusic() override;
    void PauseMusic(int theSongId) override;
    void ResumeMusic(int theSongId) override;
    void FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false) override;
    void FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004) override;
    void FadeOutAll(bool stopSong = true, double theSpeed = 0.004) override;
    void SetSongVolume(int theSongId, double theVolume) override;
    void SetSongMaxVolume(int theSongId, double theMaxVolume) override;
    bool IsPlaying(int theSongId) override;

    void SetVolume(double theVolume) override;
    void SetMusicAmplify(int theSongId, double theAmp) override; // default is 0.50
    void Update() override;

    // functions for dealing with MODs
    int GetMusicOrder(int theSongId);
};
} // namespace Sexy

#endif //__BASSMUSICINTERFACE_H__
