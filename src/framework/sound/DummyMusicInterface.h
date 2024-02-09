#ifndef __DUMMY_MUSIC_INTERFACE_H__
#define __DUMMY_MUSIC_INTERFACE_H__

#include "MusicInterface.h"

using namespace Sexy;

class DummyMusicInterface : public MusicInterface {
public:
    DummyMusicInterface() {}

    ~DummyMusicInterface() override{};

    bool LoadMusic(int, const std::string &) override { return false; }

    void PlayMusic(int, int, bool) override {}

    void StopMusic(int) override {}

    void PauseMusic(int) override {}

    void ResumeMusic(int) override {}

    void StopAllMusic() override {}

    void UnloadMusic(int) override {}

    void UnloadAllMusic() override {}

    void PauseAllMusic() override {}

    void ResumeAllMusic() override {}

    void FadeIn(int, int, double, bool) override {}

    void FadeOut(int, bool, double) override {}

    void FadeOutAll(bool, double) override {}

    void SetSongVolume(int, double) override {}

    void SetSongMaxVolume(int, double) override {}

    bool IsPlaying(int) override { return false; };

    void SetVolume(double) override {}

    void SetMusicAmplify(int, double) override {}

    void Update() override {}
};

#endif // __DUMMY_MUSIC_INTERFACE_H__
