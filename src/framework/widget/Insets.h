#ifndef __INSETS_H__
#define __INSETS_H__

namespace Sexy {
class Insets {
public:
    int mLeft;
    int mTop;
    int mRight;
    int mBottom;

public:
    Insets();
    Insets(int theLeft, int theTop, int theRight, int theBottom);
    // Insets(const Insets& theInsets);
};
} // namespace Sexy

#endif //__INSETS_H__
