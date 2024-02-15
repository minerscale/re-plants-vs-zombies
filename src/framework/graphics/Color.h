#ifndef __COLOR_H__
#define __COLOR_H__

#include "framework/Common.h"

namespace Sexy {
#pragma pack(push, 1)
struct SexyRGBA {
    unsigned char b, g, r, a;
};
#pragma pack(pop)

class Color {
public:
    int mRed;
    int mGreen;
    int mBlue;
    int mAlpha;

    static Color Black;
    static Color White;

public:
    Color();
    explicit Color(int theColor);
    Color(const int theColor, const int theAlpha);
    Color(const int theRed, const int theGreen, const int theBlue, const int theAlpha = 0xFF);
    explicit Color(const SexyRGBA &theColor);
    explicit Color(const uint8_t *theElements);
    explicit Color(const int *theElements);

    int GetRed() const;
    int GetGreen() const;
    int GetBlue() const;
    int GetAlpha() const;
    uint32_t ToInt() const;
    SexyRGBA ToRGBA() const;

    int &operator[](int theIdx);
    int operator[](int theIdx) const;
};

bool operator==(const Color &theColor1, const Color &theColor2);
bool operator!=(const Color &theColor1, const Color &theColor2);
} // namespace Sexy

#endif //__COLOR_H__
