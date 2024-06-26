#ifndef __IMAGEFONT_H__
#define __IMAGEFONT_H__

#include "Font.h"
#include "SexyAppBase.h"
#include "misc/DescParser.h"
#include <array>
#include <memory>
#include <stdexcept>
#include <unordered_map>
// #include "SharedImage.h"

namespace Sexy {
class SexyAppBase;
class Image;

using CharIntMap = std::unordered_map<char32_t, int>;
using CharIntTable = std::array<int, 128>;

class CharData {
public:
    Rect mImageRect;
    Point mOffset;
    int mWidth;
    int mOrder;
    CharIntMap mKerningOffsets{};

private:
    CharIntTable mKerningOffsetTable{};

public:
    CharData();

    inline void SetKernOffset(char32_t theChar, int theOffset) {
        mKerningOffsets[theChar] = theOffset;
        if (theChar >= 0 && theChar < mKerningOffsetTable.size()) {
            mKerningOffsetTable[theChar] = theOffset;
        }
    }

    inline int GetKernOffset(char32_t theChar) {
        if (theChar >= 0 && theChar < mKerningOffsetTable.size()) {
            return mKerningOffsetTable[theChar];
        } else {
            return mKerningOffsets[theChar];
        }
    }
};

class FontData;
using CharDataMap = std::unordered_map<char32_t, std::shared_ptr<CharData>>;
using CharDataTable = std::array<std::optional<std::shared_ptr<CharData>>, 128>;

class FontLayer {
public:
    FontData *mFontData;
    StringStringMap mExtendedInfo;
    std::string mLayerName;
    StringVector mRequiredTags;
    StringVector mExcludedTags;
    // CharData				mCharData[256];
    CharDataMap mCharDataMap{};
    CharDataTable mCharDataTable{};
    Color mColorMult;
    Color mColorAdd;
    Image *mImage;
    int mDrawMode;
    Point mOffset;
    int mSpacing;
    int mMinPointSize;
    int mMaxPointSize;
    int mPointSize;
    int mAscent;
    int mAscentPadding;     // How much space is above the avg uppercase char
    int mHeight;            //
    int mDefaultHeight;     // Max height of font character image rects
    int mLineSpacingOffset; // This plus height should get added between lines
    int mBaseOrder;
    bool mUseAlphaCorrection;

public:
    FontLayer(FontData *theFontData);
    FontLayer(const FontLayer &theFontLayer);

    CharData *GetCharData(SexyChar theChar);
};

using CharMap = std::unordered_map<SexyChar, SexyChar>;
using CharTable = std::array<std::optional<SexyChar>, 128>;
using FontLayerList = std::vector<FontLayer>;
using FontLayerMap = std::unordered_map<std::string, FontLayer *>;
using RectList = std::vector<Rect>;

class FontData : public DescParser {
public:
    bool mInitialized;
    int mRefCount;
    SexyAppBase *mApp;

    int mDefaultPointSize;
    CharMap mCharMap{};
    CharTable mCharTable{};
    FontLayerList mFontLayerList;
    FontLayerMap mFontLayerMap;

    std::string mSourceFile;
    std::string mFontErrorHeader;

public:
    bool Error(const std::string &theError) override;

    bool GetColorFromDataElement(DataElement *theElement, Color &theColor);
    bool DataToLayer(DataElement *theSource, FontLayer **theFontLayer);
    bool HandleCommand(const ListDataElement &theParams) override;
    inline void SetMappedChar(SexyChar fromChar, SexyChar toChar);

public:
    FontData();
    ~FontData() override;

    void Ref();
    void DeRef();

    bool Load(SexyAppBase *theSexyApp, const std::string &theFontDescFileName);
    bool LoadLegacy(Image *theFontImage, const std::string &theFontDescFileName);
};

using CharRectMap = std::unordered_map<SexyChar, Rect>;

class ActiveFontLayer {
public:
    FontLayer *mBaseFontLayer;

    Image *mScaledImage;
    bool mOwnsImage;
    CharRectMap mScaledCharImageRects;

public:
    ActiveFontLayer();
    ActiveFontLayer(const ActiveFontLayer &theActiveFontLayer);
    virtual ~ActiveFontLayer();
};

using ActiveFontLayerList = std::vector<ActiveFontLayer>;

class RenderCommand {
public:
    Image *mImage;
    int mDest[2];
    TRect<int> mSrc;
    int mMode;
    Color mColor;
    bool mUseAlphaCorrection;
    RenderCommand *mNext;
};

class KernTable {
    static constexpr size_t printable_ascii_size = 126 - 32;
    // To save on memory, 4 bit signed integers are used.
    using KernTable_t = std::array<std::array<int8_t, printable_ascii_size>, printable_ascii_size>;

    static constexpr KernTable_t createKernTable() {
        KernTable_t ret;

        for (auto &i : ret) {
            for (auto &j : i) {
                j = -128;
            }
        }

        return ret;
    }

    KernTable_t mKernTable = createKernTable();

public:
    void cacheKern(SexyChar theChar, SexyChar thePrevChar, int width) {
        size_t idx = theChar - 32;
        size_t prevIdx = thePrevChar - 32;

        if (idx < printable_ascii_size && prevIdx < printable_ascii_size) {
            mKernTable[prevIdx][idx] = width;
        }
    }

    inline std::optional<int8_t> findKern(SexyChar theChar, SexyChar thePrevChar) {
        size_t idx = theChar - 32;
        size_t prevIdx = thePrevChar - 32;

        if (idx >= printable_ascii_size || prevIdx >= printable_ascii_size) return {};
        int8_t ret = mKernTable[prevIdx][idx];
        if (ret == -128) return {};
        return ret;
    }

    // Here lies the beautiful compression scheme I wrote to halve the size of the kern table.
    // What I didn't realise was that CharWidthKern gave a char width not a kerning. So it was all for nothing.
    // Time to do it normal :(
    // I don't have the heart to delete it.
    /*
    void cacheKern(uint8_t theChar, uint8_t thePrevChar, int8_t kern) {
        if (kern > 7 || kern < -8) std::runtime_error("kern too big, can't fit in a signed 4 bit int: [-8, 7]");
        uint8_t idx = theChar - 32;
        uint8_t prevIdx = thePrevChar - 32;
        if (idx < printable_ascii_size && prevIdx < printable_ascii_size) {
            int8_t &aKern = mKernTable[prevIdx][idx/2];

            uint8_t store = (0b1000 * (kern < 0)) | (kern & 0b0111);
            aKern = idx % 2 ? ((aKern & 0xF0) | store) : ((store << 4) | (aKern & 0x0F));
        }
    }

    std::optional<int8_t> findKern(uint8_t theChar, uint8_t thePrevChar) {
        uint8_t idx = theChar - 32;
        uint8_t prevIdx = thePrevChar - 32;
        if (idx < printable_ascii_size && prevIdx < printable_ascii_size) {
            uint8_t stored = mKernTable[prevIdx][idx/2];

            uint8_t separated = idx % 2 ? stored & 0x0F : (stored & 0xF0) >> 4;
            return -((separated & 0b1000) != 0) * (separated & 0b0111);
        } else {
            return {};
        }
    }*/
};

class ImageFont : public _Font {
public:
    FontData *mFontData;
    int mPointSize;
    StringVector mTagVector;
    KernTable mKernTable;

    bool mActiveListValid;
    ActiveFontLayerList mActiveLayerList;
    double mScale;
    bool mForceScaledImagesWhite;

public:
    virtual void GenerateActiveFontLayers();
    virtual void DrawStringEx(
        Graphics *g, int theX, int theY, const SexyString &theString, const Color &theColor, RectList *theDrawnAreas,
        int *theWidth
    );

    virtual int RenderStringToCommandPool(
        std::vector<std::tuple<int, RenderCommand>> &aRenderCommandPool, int theX, int theY,
        const std::u32string_view &theString, const Color &theColor, RectList *theDrawnAreas
    );

public:
    ImageFont(SexyAppBase *theSexyApp, const std::string &theFontDescFileName);
    ImageFont(Image *theFontImage); // for constructing your own image font without a file descriptor
    ImageFont(const ImageFont &theImageFont);
    ~ImageFont() override;

    // Deprecated
    ImageFont(Image *theFontImage, const std::string &theFontDescFileName);
    // ImageFont(const ImageFont& theImageFont, Image* theImage);

    int CharWidth(SexyChar theChar) override;
    int CharWidthKern(SexyChar theChar, SexyChar thePrevChar) override;
    int StringWidth(const SexyString &theString) override;
    void DrawString(
        Graphics *g, int theX, int theY, const SexyString &theString, const Color &theColor, const Rect &theClipRect
    ) override;

    _Font *Duplicate() override;

    virtual void SetPointSize(int thePointSize);
    virtual int GetPointSize();
    virtual void SetScale(double theScale);
    virtual int GetDefaultPointSize();
    virtual bool AddTag(const std::string &theTagName);
    virtual bool RemoveTag(const std::string &theTagName);
    virtual bool HasTag(const std::string &theTagName);
    virtual std::string GetDefine(const std::string &theName);

    virtual void Prepare();
    SexyChar GetMappedChar(SexyChar theChar) const;
};
} // namespace Sexy

#endif //__IMAGEFONT_H__
