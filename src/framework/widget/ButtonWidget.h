#ifndef __BUTTONWIDGET_H__
#define __BUTTONWIDGET_H__

#include "Widget.h"

namespace Sexy {
class Image;
class ButtonListener;

class ButtonWidget : public Widget {
public:
    enum { BUTTON_LABEL_LEFT = -1, BUTTON_LABEL_CENTER, BUTTON_LABEL_RIGHT };

    enum {
        COLOR_LABEL,
        COLOR_LABEL_HILITE,
        COLOR_DARK_OUTLINE,
        COLOR_LIGHT_OUTLINE,
        COLOR_MEDIUM_OUTLINE,
        COLOR_BKG,
        NUM_COLORS
    };

    int mId;
    SexyString mLabel;
    int mLabelJustify;
    _Font *mFont;
    Image *mButtonImage;
    Image *mOverImage;
    Image *mDownImage;
    Image *mDisabledImage;
    Rect mNormalRect;
    Rect mOverRect;
    Rect mDownRect;
    Rect mDisabledRect;

    bool mInverted;
    bool mBtnNoDraw;
    bool mFrameNoDraw;
    ButtonListener *mButtonListener;

    double mOverAlpha;
    double mOverAlphaSpeed;
    double mOverAlphaFadeInSpeed;

    bool HaveButtonImage(Image *theImage, const Rect &theRect);
    virtual void DrawButtonImage(Graphics *g, Image *theImage, const Rect &theRect, int x, int y);

public:
    ButtonWidget(int theId, ButtonListener *theButtonListener);
    ~ButtonWidget() override;

    virtual void SetFont(_Font *theFont);
    virtual bool IsButtonDown();
    void Draw(Graphics *g) override;
    void SetDisabled(bool isDisabled) override;
    void MouseEnter() override;
    void MouseLeave() override;
    void MouseMove(int theX, int theY) override;
    void MouseDown(int theX, int theY, int theClickCount) override { Widget::MouseDown(theX, theY, theClickCount); }
    void MouseDown(int theX, int theY, int theBtnNum, int theClickCount) override;
    void MouseUp(int theX, int theY) override { Widget::MouseUp(theX, theY); }
    void MouseUp(int theX, int theY, int theClickCount) override { Widget::MouseUp(theX, theY, theClickCount); }
    void MouseUp(int theX, int theY, int theBtnNum, int theClickCount) override;
    void Update() override;
};
} // namespace Sexy

#endif //__BUTTONWIDGET_H__
