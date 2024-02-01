#ifndef __IMAGELIB_H__
#define __IMAGELIB_H__

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "framework/misc/ResourceManager.h"

namespace ImageLib {

class Image {
public:
    int mWidth = 0;
    int mHeight = 0;
    std::unique_ptr<uint32_t[]> mBits = nullptr;

    Image(int width, int height)
        : mWidth(width), mHeight(height), mBits(std::make_unique<uint32_t[]>(mWidth * mHeight)) {
        memset(mBits.get(), 0, mWidth * mHeight * sizeof(uint32_t));
    }

    Image(int width, int height, std::unique_ptr<uint32_t[]> bits)
        : mWidth(width), mHeight(height), mBits(std::move(bits)) {}
};

bool WriteJPEGImage(const std::string &theFileName, Image *theImage);
bool WritePNGImage(const std::string &theFileName, Image *theImage);
bool WriteTGAImage(const std::string &theFileName, Image *theImage);
bool WriteBMPImage(const std::string &theFileName, Image *theImage);
extern int gAlphaComposeColor;
extern bool gAutoLoadAlpha;
extern bool gIgnoreJPEG2000Alpha; // I've noticed alpha in jpeg2000's that shouldn't have alpha so this defaults to true

std::unique_ptr<ImageLib::Image> GetImage(const Sexy::ResourceManager::ImageRes &theFilename, bool lookForAlphaImage);

// void InitJPEG2000();
// void CloseJPEG2000();
// void SetJ2KCodecKey(const std::string& theKey);

} // namespace ImageLib

#endif //__IMAGELIB_H__
