#include "misc/ResourceManager.h"
#include "misc/fcaseopen.h"
#include <limits>
#include <memory>
#include <stdexcept>
#define XMD_H

#include "ImageLib.h"

#include <SDL.h>
#include <SDL_image.h>

#include <math.h>

using namespace ImageLib;

std::unique_ptr<Image> GetImageWithSDL(const std::string &theFileName) {
    std::string corrected_path = casepath(theFileName.c_str());
    if (corrected_path == "") return nullptr;

    SDL_Surface *aSurface = IMG_Load(corrected_path.c_str());

    if (!aSurface) return nullptr;

    auto aSurface32 = SDL_ConvertSurfaceFormat(aSurface, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(aSurface);

    if (!aSurface32) return nullptr;

    auto anImage = std::make_unique<Image>(aSurface32->w, aSurface32->h);

    auto bufferSize = aSurface32->w * aSurface32->h;

    // Copy the pixels
    SDL_memcpy(anImage->mBits.get(), aSurface32->pixels, bufferSize * sizeof(uint32_t));

    SDL_FreeSurface(aSurface32);

    return anImage;
}

bool ImageLib::WriteJPEGImage(const std::string &theFileName, Image *theImage) {
    auto aSurface = SDL_CreateRGBSurfaceFrom(
        theImage->mBits.get(), theImage->mWidth, theImage->mHeight, 32, theImage->mWidth * 4, 0x00FF0000, 0x0000FF00,
        0x000000FF, 0xFF000000
    );
    if (aSurface == NULL) return false;
    auto aResult = IMG_SaveJPG(aSurface, theFileName.c_str(), 80);
    if (aResult != 0) return false;
    return true;
}

bool ImageLib::WritePNGImage(const std::string &theFileName, Image *theImage) {
    auto aSurface = SDL_CreateRGBSurfaceFrom(
        theImage->mBits.get(), theImage->mWidth, theImage->mHeight, 32, theImage->mWidth * 4, 0x00FF0000, 0x0000FF00,
        0x000000FF, 0xFF000000
    );
    if (aSurface == NULL) return false;
    auto aResult = IMG_SavePNG(aSurface, theFileName.c_str());
    if (aResult != 0) return false;
    return true;
}

bool ImageLib::WriteTGAImage(const std::string &theFileName, Image *theImage) {
    FILE *aTGAFile = fopen(theFileName.c_str(), "wb");
    if (aTGAFile == NULL) return false;

    unsigned char aHeaderIDLen = 0;
    fwrite(&aHeaderIDLen, sizeof(unsigned char), 1, aTGAFile);

    unsigned char aColorMapType = 0;
    fwrite(&aColorMapType, sizeof(unsigned char), 1, aTGAFile);

    unsigned char anImageType = 2;
    fwrite(&anImageType, sizeof(unsigned char), 1, aTGAFile);

    uint16_t aFirstEntryIdx = 0;
    fwrite(&aFirstEntryIdx, sizeof(uint16_t), 1, aTGAFile);

    uint16_t aColorMapLen = 0;
    fwrite(&aColorMapLen, sizeof(uint16_t), 1, aTGAFile);

    unsigned char aColorMapEntrySize = 0;
    fwrite(&aColorMapEntrySize, sizeof(unsigned char), 1, aTGAFile);

    uint16_t anXOrigin = 0;
    fwrite(&anXOrigin, sizeof(uint16_t), 1, aTGAFile);

    uint16_t aYOrigin = 0;
    fwrite(&aYOrigin, sizeof(uint16_t), 1, aTGAFile);

    uint16_t anImageWidth = theImage->mWidth;
    fwrite(&anImageWidth, sizeof(uint16_t), 1, aTGAFile);

    uint16_t anImageHeight = theImage->mHeight;
    fwrite(&anImageHeight, sizeof(uint16_t), 1, aTGAFile);

    unsigned char aBitCount = 32;
    fwrite(&aBitCount, sizeof(unsigned char), 1, aTGAFile);

    unsigned char anImageDescriptor = 8 | (1 << 5);
    fwrite(&anImageDescriptor, sizeof(unsigned char), 1, aTGAFile);

    fwrite(theImage->mBits.get(), 4, theImage->mWidth * theImage->mHeight, aTGAFile);

    fclose(aTGAFile);

    return true;
}

typedef struct tagBITMAPFILEHEADER {
    uint16_t bfType;
    unsigned int bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef enum {
    BI_RGB = 0x0000,
    BI_RLE8 = 0x0001,
    BI_RLE4 = 0x0002,
    BI_BITFIELDS = 0x0003,
    BI_JPEG = 0x0004,
    BI_PNG = 0x0005,
    BI_CMYK = 0x000B,
    BI_CMYKRLE8 = 0x000C,
    BI_CMYKRLE4 = 0x000D
} Compression;

bool ImageLib::WriteBMPImage(const std::string &theFileName, Image *theImage) {
    FILE *aFile = fopen(theFileName.c_str(), "wb");
    if (aFile == NULL) return false;

    BITMAPFILEHEADER aFileHeader;
    BITMAPINFOHEADER aHeader;

    memset(&aFileHeader, 0, sizeof(aFileHeader));
    memset(&aHeader, 0, sizeof(aHeader));

    int aNumBytes = theImage->mWidth * theImage->mHeight * 4;

    aFileHeader.bfType = ('M' << 8) | 'B';
    aFileHeader.bfSize = sizeof(aFileHeader) + sizeof(aHeader) + aNumBytes;
    aFileHeader.bfOffBits = sizeof(aHeader);

    aHeader.biSize = sizeof(aHeader);
    aHeader.biWidth = theImage->mWidth;
    aHeader.biHeight = theImage->mHeight;
    aHeader.biPlanes = 1;
    aHeader.biBitCount = 32;
    aHeader.biCompression = BI_RGB;

    fwrite(&aFileHeader, sizeof(aFileHeader), 1, aFile);
    fwrite(&aHeader, sizeof(aHeader), 1, aFile);
    unsigned int *aRow = theImage->mBits.get() + (theImage->mHeight - 1) * theImage->mWidth;
    int aRowSize = theImage->mWidth * 4;
    (void)aRowSize; // Unused
    for (int i = 0; i < theImage->mHeight; i++, aRow -= theImage->mWidth)
        fwrite(aRow, 4, theImage->mWidth, aFile);

    fclose(aFile);
    return true;
}

int ImageLib::gAlphaComposeColor = 0xFFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

constexpr double linearToSRGB(double theLinearValue) {
    return theLinearValue <= 0.0031308 ? theLinearValue * 12.92 : pow(theLinearValue, 1.0 / 2.4) * 1.055 - 0.055;
}

constexpr double sRGBToLinear(double thesRGBValue) {
    return thesRGBValue <= 0.04045 ? thesRGBValue / 12.92 : pow((thesRGBValue + 0.055) / 1.055, 2.4);
}

template <typename T, size_t N> constexpr std::array<T, N> createLUT(auto fn) {
    std::array<T, N> ret;

    for (size_t i = 0; i < N; ++i) {
        // Subtracting one here ensures highest number is 1
        double v = (double)i / (N - 1);
        // Use the full range of our type given to us for max precision.
        ret[i] = std::numeric_limits<T>::max() * fn(v);
    }

    return ret;
}

const auto sRGBToLinearLut = createLUT<uint16_t, 256>(sRGBToLinear);
const auto linearToSRGBLut = createLUT<uint8_t, 1024>(linearToSRGB);

std::unique_ptr<ImageLib::Image> GetAnImage(const std::string &theFilename) {
    int aLastDotPos = theFilename.rfind('.');
    int aLastSlashPos = std::max((int)theFilename.rfind('\\'), (int)theFilename.rfind('/'));

    std::string anExt;
    std::string aFilename;

    if (aLastDotPos > aLastSlashPos) {
        anExt = theFilename.substr(aLastDotPos, theFilename.length() - aLastDotPos);
        aFilename = theFilename.substr(0, aLastDotPos);
    } else aFilename = theFilename;

    std::unique_ptr<Image> anImage = nullptr;

    if ((anImage == NULL) && ((strcasecmp(anExt.c_str(), ".tga") == 0) || (anExt.length() == 0)))
        anImage = GetImageWithSDL(aFilename + ".tga");
    if ((anImage == NULL) && ((strcasecmp(anExt.c_str(), ".jpg") == 0) || (anExt.length() == 0)))
        anImage = GetImageWithSDL(aFilename + ".jpg");
    if ((anImage == NULL) && ((strcasecmp(anExt.c_str(), ".png") == 0) || (anExt.length() == 0)))
        anImage = GetImageWithSDL(aFilename + ".png");
    if ((anImage == NULL) && ((strcasecmp(anExt.c_str(), ".gif") == 0) || (anExt.length() == 0)))
        anImage = GetImageWithSDL(aFilename + ".gif");
    if ((anImage == NULL) && (strcasecmp(anExt.c_str(), ".j2k") == 0)) anImage = GetImageWithSDL(aFilename + ".j2k");
    if ((anImage == NULL) && (strcasecmp(anExt.c_str(), ".jp2") == 0)) anImage = GetImageWithSDL(aFilename + ".jp2");

    return anImage;
}

std::unique_ptr<ImageLib::Image> GetAlphaImage(const std::string &theFilename) {
    std::unique_ptr<Image> anAlphaImage = nullptr;
    // Check ImageName_
    anAlphaImage = GetAnImage(theFilename + "_");

    // Check _ImageName
    if (!anAlphaImage) {
        int aLastSlashPos = std::max((int)theFilename.rfind('\\'), (int)theFilename.rfind('/'));

        anAlphaImage = GetAnImage(
            theFilename.substr(0, aLastSlashPos + 1) + "_" +
            theFilename.substr(aLastSlashPos + 1, theFilename.length() - aLastSlashPos - 1)
        );
    }

    return anAlphaImage;
}

std::unique_ptr<ImageLib::Image>
composeAlphaImage(std::unique_ptr<ImageLib::Image> theImage, std::unique_ptr<ImageLib::Image> theAlphaImage) {
    if (theImage != NULL) {
        if ((theImage->mWidth == theAlphaImage->mWidth) && (theImage->mHeight == theAlphaImage->mHeight)) {
            uint32_t *aBits1 = theImage->mBits.get();
            uint32_t *aBits2 = theAlphaImage->mBits.get();
            int aSize = theImage->mWidth * theImage->mHeight;

            for (int i = 0; i < aSize; i++) {
                *aBits1 = (*aBits1 & 0x00FFFFFF) | ((*aBits2 & 0xFF) << 24);
                //*aBits1 = (*aBits1 & 0x00FFFFFF) | (sRGBTolinearLut[(*aBits2 & 0xFF)] << 24);
                ++aBits1;
                ++aBits2;
            }
        }
    } else if (gAlphaComposeColor == 0xFFFFFF) {
        theImage = std::move(theAlphaImage);

        uint32_t *aBits1 = theImage->mBits.get();

        int aSize = theImage->mWidth * theImage->mHeight;
        for (int i = 0; i < aSize; i++) {
            *aBits1 = (0x00FFFFFF) | ((*aBits1 & 0xFF) << 24);
            //*aBits1 = (0x00FFFFFF) | (sRGBTolinearLut[(*aBits1 & 0xFF)] << 24);
            ++aBits1;
        }
    } else {
        const int aColor = gAlphaComposeColor;
        theImage = std::move(theAlphaImage);

        uint32_t *aBits1 = theImage->mBits.get();

        int aSize = theImage->mWidth * theImage->mHeight;
        for (int i = 0; i < aSize; i++) {
            *aBits1 = aColor | ((*aBits1 & 0xFF) << 24);
            //*aBits1 = aColor | (sRGBTolinearLut[(*aBits1 & 0xFF)] << 24);
            ++aBits1;
        }
    }

    return theImage;
}

std::unique_ptr<ImageLib::Image>
ImageLib::GetImage(const Sexy::ResourceManager::ImageRes &theRes, bool lookForAlphaImage) {
    const std::string &theFilename = theRes.mPath;
    if (!gAutoLoadAlpha) lookForAlphaImage = false;

    std::unique_ptr<Image> anImage = GetAnImage(theFilename);

    // Check for alpha images
    std::unique_ptr<Image> anAlphaImage = nullptr;
    if (lookForAlphaImage) {
        anAlphaImage = GetAlphaImage(theFilename);

        if (anAlphaImage != nullptr) {
            anImage = composeAlphaImage(std::move(anImage), std::move(anAlphaImage));
        }
    }

    if (!theRes.mAlphaImage.empty()) {
        std::unique_ptr<Image> anAlphaImage = GetAnImage(theRes.mAlphaGridImage);
        if (anAlphaImage == nullptr) throw std::runtime_error("Failed to load image: " + theRes.mAlphaGridImage);

        std::unique_ptr<Image> anAlphaImagesAlphaImage = GetAlphaImage(theRes.mAlphaGridImage);
        if (anAlphaImagesAlphaImage)
            anAlphaImage = composeAlphaImage(std::move(anAlphaImage), std::move(anAlphaImagesAlphaImage));

        if (anAlphaImage->mWidth != anImage->mWidth || anAlphaImage->mHeight != anImage->mHeight)
            throw std::runtime_error("AlphaImage size mismatch between " + theRes.mPath + " and " + theRes.mAlphaImage);

        uint32_t *aBits1 = anImage->mBits.get();
        uint32_t *aBits2 = anAlphaImage->mBits.get();
        int aSize = anImage->mWidth * anImage->mHeight;

        for (int i = 0; i < aSize; i++) {
            *aBits1 = (*aBits1 & 0x00FFFFFF) | ((*aBits2 & 0xFF) << 24);
            ++aBits1;
            ++aBits2;
        }
    }

    if (!theRes.mAlphaGridImage.empty()) {
        std::unique_ptr<Image> anAlphaImage = GetAnImage(theRes.mAlphaGridImage);
        if (anAlphaImage == nullptr) throw std::runtime_error("Failed to load image: " + theRes.mAlphaGridImage);

        std::unique_ptr<Image> anAlphaImagesAlphaImage = GetAlphaImage(theRes.mAlphaGridImage);
        if (anAlphaImagesAlphaImage)
            anAlphaImage = composeAlphaImage(std::move(anAlphaImage), std::move(anAlphaImagesAlphaImage));

        int aNumRows = theRes.mRows;
        int aNumCols = theRes.mCols;

        int aCelWidth = anImage->mWidth / aNumCols;
        int aCelHeight = anImage->mHeight / aNumRows;

        if (anAlphaImage->mWidth != aCelWidth || anAlphaImage->mHeight != aCelHeight)
            throw std::runtime_error(
                "GridAlphaImage size mismatch between " + theRes.mPath + "and" + theRes.mAlphaGridImage
            );

        uint32_t *aMasterRowPtr = anImage->mBits.get();
        for (int i = 0; i < aNumRows; i++) {
            uint32_t *aMasterColPtr = aMasterRowPtr;
            for (int j = 0; j < aNumCols; j++) {
                uint32_t *aRowPtr = aMasterColPtr;
                uint32_t *anAlphaBits = anAlphaImage->mBits.get();
                for (int y = 0; y < aCelHeight; y++) {
                    uint32_t *aDestPtr = aRowPtr;
                    for (int x = 0; x < aCelWidth; x++) {
                        *aDestPtr = (*aDestPtr & 0x00FFFFFF) | ((*anAlphaBits & 0xFF) << 24);
                        ++anAlphaBits;
                        ++aDestPtr;
                    }
                    aRowPtr += anImage->mWidth;
                }

                aMasterColPtr += aCelWidth;
            }
            aMasterRowPtr += aCelHeight * anImage->mWidth;
        }
    }

    // Premultiply the alpha channel.
    if (anImage != NULL) {
        // Premultiply alpha
        uint32_t *aBitsPtr = anImage->mBits.get();

        for (int y = 0; y < anImage->mHeight; y++) {
            for (int x = 0; x < anImage->mWidth; x++) {
                uint32_t pixel = *aBitsPtr;
                // sRGBToLinearLut
                uint32_t r = sRGBToLinearLut[(pixel & 0x00FF0000) >> 16];
                uint32_t g = sRGBToLinearLut[(pixel & 0x0000FF00) >> 8];
                uint32_t b = sRGBToLinearLut[pixel & 0x000000FF];
                uint32_t alpha = sRGBToLinearLut[((*aBitsPtr & 0xFF000000) >> 24)];
                *aBitsPtr =
                    // 16 bit fixed * 16 bit fixed = 32 bit fixed.
                    // Shift 22 places to fit in a 10 bit LUT.
                    linearToSRGBLut[((alpha * r) >> 22)] << 16 | linearToSRGBLut[((alpha * g) >> 22)] << 8 |
                    linearToSRGBLut[((alpha * b) >> 22)] | (*aBitsPtr & 0xFF000000);

                ++aBitsPtr;
            }
        }
    }

    return anImage;
}
