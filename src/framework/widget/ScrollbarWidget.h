// ScrollbarWidget.h: interface for the ScrollbarWidget class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SCROLLBARWIDGET_H__
#define __SCROLLBARWIDGET_H__

#include "ButtonListener.h"
#include "Widget.h"

namespace Sexy {
class ScrollListener;
class ScrollbuttonWidget;

class ScrollbarWidget : public Widget, public ButtonListener {
public:
    enum { UPDATE_MODE_IDLE = 0, UPDATE_MODE_PGUP, UPDATE_MODE_PGDN };

public:
    ScrollbuttonWidget *mUpButton;
    ScrollbuttonWidget *mDownButton;

    bool mInvisIfNoScroll;

public:
    int mId;

    double mValue;
    double mMaxValue;
    double mPageSize;
    bool mHorizontal;

    bool mPressedOnThumb;
    int mMouseDownThumbPos;
    int mMouseDownX;
    int mMouseDownY;

    int mUpdateMode;
    int mUpdateAcc;
    int mButtonAcc;
    int mLastMouseX;
    int mLastMouseY;
    ScrollListener *mScrollListener;

public:
    ScrollbarWidget(int theId, ScrollListener *theScrollListener);
    ~ScrollbarWidget() override;

    virtual void SetInvisIfNoScroll(bool invisIfNoScroll);
    virtual void SetMaxValue(double theNewMaxValue);
    virtual void SetPageSize(double theNewPageSize);
    virtual void SetValue(double theNewValue);
    virtual void SetHorizontal(bool isHorizontal);

    virtual void ResizeScrollbar(int theX, int theY, int theWidth, int theHeight);
    virtual bool AtBottom();
    virtual void GoToBottom();
    virtual void DrawThumb(Graphics *g, int theX, int theY, int theWidth, int theHeight);
    virtual int GetTrackSize();
    virtual int GetThumbSize();
    virtual int GetThumbPosition();
    void Draw(Graphics *g) override;
    virtual void ClampValue();
    virtual void SetThumbPosition(int thePosition);
    void ButtonPress(int theId) override;
    void ButtonDepress(int theId) override;
    void ButtonDownTick(int theId) override;
    void Update() override;
    virtual int ThumbCompare(int x, int y);
    void MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
    void MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
    void MouseUp(int x, int y) override { Widget::MouseUp(x, y); }
    void MouseUp(int x, int y, int theBtnNum, int theClickCount) override;
    void MouseDrag(int x, int y) override;
};
} // namespace Sexy

#endif // __SCROLLBARWIDGET_H__
