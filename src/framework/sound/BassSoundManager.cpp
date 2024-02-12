#include "BassSoundManager.h"
#include "Common.h"
#include "paklib/PakInterface.h"
#include <bass.h>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <optional>

namespace Sexy {
BassSoundManager::BassSoundManager(HWND theHWnd) { BassMusicInterface::InitBass(theHWnd); }

bool BassSoundManager::LoadCompatibleSound(unsigned int theSfxID, const std::string &theFilename) {
    PFILE *aFile = p_fopen(theFilename.c_str(), "rb");
    if (aFile == nullptr) return false;

    p_fseek(aFile, 0, SEEK_END);
    const size_t aLength = p_ftell(aFile);
    p_fseek(aFile, 0, SEEK_SET);

    const auto aBuf = static_cast<char *>(malloc(aLength));
    p_fread(aBuf, aLength, 1, aFile);
    p_fclose(aFile);

    mSourceSounds[theSfxID] = std::make_optional(BASS_SampleLoad(true, aBuf, 0, aLength, MAX_CHANNELS, 0));

    free(aBuf);
    return true;
}

struct WavHeader {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    uint32_t wav_size;   // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4];      // Contains "fmt " (includes trailing space)
    uint32_t fmt_chunk_size; // Should be 16 for PCM
    short audio_format;      // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;     // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth;        // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    uint32_t data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
                         // uint8_t bytes[]; // Remainder of wave file is bytes
};

// From g711.c
inline uint16_t Snack_Mulaw2Lin(uint8_t u_val) {
    constexpr uint8_t QUANT_MASK = 0xf;
    constexpr uint16_t BIAS = 0x84;
    constexpr uint32_t SEG_MASK = 0x70;
    constexpr uint8_t SEG_SHIFT = 4;
    constexpr uint8_t SIGN_BIT = 0x80;

    /* Complement to obtain normal u-law value. */
    u_val = ~u_val;

    /*
     * Extract and bias the quantization bits. Then
     * shift up by the segment number and subtract out the bias.
     */
    uint16_t t = ((u_val & QUANT_MASK) << 3) + BIAS;
    t <<= (static_cast<uint32_t>(u_val) & SEG_MASK) >> SEG_SHIFT;

    return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

bool BassSoundManager::LoadAUSound(unsigned int theSfxID, const std::string &theFilename) {
    PFILE *fp = p_fopen(theFilename.c_str(), "rb");

    if (fp == nullptr) return false;

    char aHeaderId[4];
    p_fread(aHeaderId, 1, 4, fp);
    if ((!strncmp(aHeaderId, ".snd", 4)) == 0) return false;

    uint32_t aHeaderSize;
    p_fread(&aHeaderSize, 4, 1, fp);
    aHeaderSize = LONG_BIGE_TO_NATIVE(aHeaderSize);

    uint32_t aDataSize;
    p_fread(&aDataSize, 4, 1, fp);
    aDataSize = LONG_BIGE_TO_NATIVE(aDataSize);

    uint32_t anEncoding;
    p_fread(&anEncoding, 4, 1, fp);
    anEncoding = LONG_BIGE_TO_NATIVE(anEncoding);

    uint32_t aSampleRate;
    p_fread(&aSampleRate, 4, 1, fp);
    aSampleRate = LONG_BIGE_TO_NATIVE(aSampleRate);

    uint32_t aChannelCount;
    p_fread(&aChannelCount, 4, 1, fp);
    aChannelCount = LONG_BIGE_TO_NATIVE(aChannelCount);

    p_fseek(fp, aHeaderSize, SEEK_SET);

    bool ulaw = false;

    uint32_t aSrcBitCount = 8;
    uint32_t aBitCount = 16;
    switch (anEncoding) {
    case 1:
        aSrcBitCount = 8;
        aBitCount = 16;
        ulaw = true;
        break;
    case 2:
        aSrcBitCount = 8;
        aBitCount = 8;
        break;

        /*
        Support these formats?

        case 3:
            aBitCount = 16;
            break;
        case 4:
            aBitCount = 24;
            break;
        case 5:
            aBitCount = 32;
            break;*/

    default: return false;
    }

    const uint32_t aDestSize = aDataSize * (aBitCount / aSrcBitCount);

    const auto aDestHeader = static_cast<WavHeader *>(calloc(1, sizeof(WavHeader) + aDestSize));

    *aDestHeader = WavHeader{
        {'R', 'I', 'F', 'F'},
        static_cast<uint32_t>(aDestSize + sizeof(WavHeader) - offsetof(WavHeader, wave_header)),
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        offsetof(WavHeader, data_header) - offsetof(WavHeader, audio_format),
        1,
        static_cast<short>(aChannelCount),
        aSampleRate,
        (aSampleRate * aBitCount * aChannelCount) / 8,
        static_cast<short>((aBitCount * aChannelCount) / 8),
        static_cast<short>(aBitCount),
        {'d', 'a', 't', 'a'},
        aDestSize,
    };

    const auto aDestBuffer = reinterpret_cast<short *>(reinterpret_cast<char *>(aDestHeader) + sizeof(WavHeader));

    if (ulaw) {
        const auto aSrcBuffer = new uint8_t[aDataSize];

        const size_t aReadSize = p_fread(aSrcBuffer, 1, aDataSize, fp);
        p_fclose(fp);
        if (aReadSize != aDataSize) return false;

        for (uint32_t i = 0; i < aDataSize; i++) {
            aDestBuffer[i] = Snack_Mulaw2Lin(aSrcBuffer[i]);
        }

        delete[] aSrcBuffer;
    } else {
        const size_t aReadSize = p_fread(aDestBuffer, 1, aDataSize, fp);
        p_fclose(fp);
        if (aReadSize != aDataSize) return false;
    }

    mSourceSounds[theSfxID] =
        std::make_optional(BASS_SampleLoad(true, aDestHeader, 0, sizeof(WavHeader) + aDestSize, MAX_CHANNELS, 0));

    free(aDestHeader);

    return true;
}

bool BassSoundManager::LoadSound(unsigned int theSfxID, const std::string &theFilename) {
    if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS)) return false;

    ReleaseSound(theSfxID);

    mSourceFileNames[theSfxID] = theFilename;

    const std::string aFilename = theFilename;
    // std::string aCachedName;

    /* Disabling caching because caching is hard.
    if ((aFilename.length() > 2) && (aFilename[0] != '\\') && (aFilename[0] != '/') &&
        (aFilename[1] != ':'))
    {
        // Not an absolute path
        aCachedName = GetAppDataFolder() + "cached\\" + aFilename + ".wav";
        if (LoadCompatibleSound(theSfxID, aCachedName))
            return true;
        MkDir(GetFileDir(aCachedName));
    }*/

    if (LoadCompatibleSound(theSfxID, aFilename + ".wav")) return true;

    if (LoadCompatibleSound(theSfxID, aFilename + ".ogg")) {
        // WriteWAV(theSfxID, aCachedName, aFilename + ".ogg");
        return true;
    }

    if (LoadAUSound(theSfxID, aFilename + ".au")) {
        // WriteWAV(theSfxID, aCachedName, aFilename + ".au");
        return true;
    }

    return false;
}

int BassSoundManager::LoadSound(const std::string &theFilename) {
    int i;
    for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
        if (mSourceFileNames[i] == theFilename) return i;

    for (i = MAX_SOURCE_SOUNDS - 1; i >= 0; i--) {
        if (!mSourceSounds[i].has_value()) {
            if (!LoadSound(i, theFilename)) return -1;
            else return i;
        }
    }

    return -1;
}

void BassSoundManager::ReleaseSound(unsigned int theSfxID) {
    if (Exists(theSfxID)) {
        BASS_SampleFree(mSourceSounds[theSfxID].value());
        mSourceSounds[theSfxID].reset();
        mSourceFileNames[theSfxID] = "";
    }
}

void BassSoundManager::ReleaseSounds() {
    for (unsigned int i = 0; i < MAX_SOURCE_SOUNDS; ++i) {
        ReleaseSound(i);
    }
}

inline bool BassSoundManager::Exists(unsigned int theSfxID) const {
    return (theSfxID < MAX_SOURCE_SOUNDS) && (mSourceSounds[theSfxID].has_value());
}

void BassSoundManager::SetVolume(double theVolume) {
    BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, static_cast<int>(theVolume * 10000));
}

double BassSoundManager::PanDBToNorm(int dbpan) {
    auto sign = [](int val) { return (0 < val) - (val < 0); };
    return sign(dbpan) * pow(10.0, -abs(dbpan) / 2000.0);
};

void BassSoundManager::SetBaseVolumeAndPan(
    HSAMPLE theSample, std::optional<double> theBaseVolume, std::optional<int> theBasePan
) {
    BASS_SAMPLE info;
    BASS_SampleGetInfo(theSample, &info);

    if (theBasePan.has_value()) {
        auto sign = [](int val) { return (0 < val) - (val < 0); };
        info.pan = sign(theBasePan.value()) * pow(10.0, -abs(theBasePan.value()) / 2000.0);
    }

    if (theBaseVolume.has_value()) {
        info.volume = theBaseVolume.value();
    }

    BASS_SampleSetInfo(theSample, &info);
}

bool BassSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume) {
    if (!Exists(theSfxID)) return false;

    SetBaseVolumeAndPan(mSourceSounds[theSfxID].value(), theBaseVolume, {});

    return true;
}

bool BassSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan) {
    if (!Exists(theSfxID)) return false;

    SetBaseVolumeAndPan(mSourceSounds[theSfxID].value(), {}, theBasePan);

    return true;
}

int BassSoundManager::GetFreeSoundId() {
    for (int i = 0; i < MAX_SOURCE_SOUNDS; i++) {
        if (!mSourceSounds[i].has_value()) return i;
    }

    return -1;
}

int BassSoundManager::GetNumSounds() {
    int aCount = 0;
    for (int i = 0; i < MAX_SOURCE_SOUNDS; i++) {
        if (mSourceSounds[i].has_value()) aCount++;
    }

    return aCount;
}

void BassSoundManager::ReleaseFreeChannels() {
    for (int i = 0; i < MAX_CHANNELS; i++)
        if (mPlayingSounds[i].has_value() && mPlayingSounds[i].value()->IsReleased()) {
            mPlayingSounds[i].reset();
        }
}

int BassSoundManager::FindFreeChannel() {
    static auto timer = std::chrono::high_resolution_clock::now();
    const auto now = std::chrono::high_resolution_clock::now();
    if (now - timer > std::chrono::duration<double>(1)) {
        ReleaseFreeChannels();
        timer = now;
    }

    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (!mPlayingSounds[i].has_value()) return i;

        if (mPlayingSounds[i].value()->IsReleased()) {
            mPlayingSounds[i].reset();
            return i;
        }
    }

    return -1;
}

SoundInstance *BassSoundManager::GetSoundInstance(unsigned int theSfxID) {
    if (!Exists(theSfxID)) return nullptr;

    const int aFreeChannel = FindFreeChannel();
    if (aFreeChannel < 0) return nullptr;

    mPlayingSounds[aFreeChannel] = std::make_unique<BassSoundInstance>(mSourceSounds[theSfxID].value());

    return mPlayingSounds[aFreeChannel].value().get();
}
} // Namespace Sexy
