#ifndef __WIDGETMANAGER_H__
#define __WIDGETMANAGER_H__

#include "WidgetContainer.h"
#include "framework/Common.h"
#include "framework/misc/KeyCodes.h"

namespace Sexy {
class Widget;
class Image;
// class MemoryImage;
class SexyAppBase;
class Graphics;

using WidgetList = std::list<Widget *>;

enum {
    WIDGETFLAGS_UPDATE = 1,
    WIDGETFLAGS_MARK_DIRTY = 2,
    WIDGETFLAGS_DRAW = 4,
    WIDGETFLAGS_CLIP = 8,
    WIDGETFLAGS_ALLOW_MOUSE = 16,
    WIDGETFLAGS_ALLOW_FOCUS = 32,
};

class PreModalInfo {
public:
    Widget *mBaseModalWidget;
    Widget *mPrevBaseModalWidget;
    Widget *mPrevFocusWidget;
    FlagsMod mPrevBelowModalFlagsMod;
};

using PreModalInfoList = std::list<PreModalInfo>;

using DeferredOverlayVector = std::vector<std::pair<Widget *, int>>;

class WidgetManager : public WidgetContainer {
public:
    Widget *mDefaultTab;

    Graphics *mCurG;
    SexyAppBase *mApp;
    Image *mImage;
    //	MemoryImage*			mTransientImage;
    bool mLastHadTransients;
    Widget *mPopupCommandWidget;
    DeferredOverlayVector mDeferredOverlayWidgets;
    int mMinDeferredOverlayPriority;

    bool mHasFocus;
    Widget *mFocusWidget;
    Widget *mLastDownWidget;
    Widget *mOverWidget;
    Widget *mBaseModalWidget;
    FlagsMod mLostFocusFlagsMod;
    FlagsMod mBelowModalFlagsMod;
    FlagsMod mDefaultBelowModalFlagsMod;
    PreModalInfoList mPreModalInfoList;
    Rect mMouseDestRect;
    Rect mMouseSourceRect;
    bool mMouseIn;
    int mLastMouseX;
    int mLastMouseY;
    int mDownButtons;
    int mActualDownButtons;
    int mLastInputUpdateCnt;

    bool mKeyDown[0xFF];
    int mLastDownButtonId;

    int mWidgetFlags;

protected:
    int GetWidgetFlags() const;
    static void MouseEnter(Widget *theWidget);
    static void MouseLeave(Widget *theWidget);

protected:
    void SetBaseModal(Widget *theWidget, const FlagsMod &theBelowFlagsMod);

public:
    WidgetManager(SexyAppBase *theApplet);
    ~WidgetManager() override;

    static void FreeResources();
    void AddBaseModal(Widget *theWidget, const FlagsMod &theBelowFlagsMod);
    void AddBaseModal(Widget *theWidget);
    void RemoveBaseModal(Widget *theWidget);
    void Resize(const Rect &theMouseDestRect, const Rect &theMouseSourceRect);
    void DisableWidget(Widget *theWidget) override;
    Widget *GetAnyWidgetAt(int x, int y, int *theWidgetX, int *theWidgetY);
    Widget *GetWidgetAt(int x, int y, int *theWidgetX, int *theWidgetY);
    void SetFocus(Widget *aWidget) override;
    void GotFocus();
    void LostFocus();
    void InitModalFlags(ModalFlags *theModalFlags);
    void DrawWidgetsTo(Graphics *g);
    void DoMouseUps(Widget *theWidget, uint32_t theDownCode) const;
    void DoMouseUps();
    void DeferOverlay(Widget *theWidget, int thePriority);
    void FlushDeferredOverlayWidgets(int theMaxPriority);

    bool DrawScreen();
    bool UpdateFrame();
    bool UpdateFrameF(float theFrac);
    void SetPopupCommandWidget(Widget *theList);
    void RemovePopupCommandWidget();
    void MousePosition(int x, int y);
    void RehupMouse();
    void RemapMouse(int &theX, int &theY) const;
    bool MouseUp(int x, int y, int theClickCount);
    bool MouseDown(int x, int y, int theClickCount);
    bool MouseMove(int x, int y);
    bool MouseDrag(int x, int y);
    bool MouseExit(int x, int y);
    void MouseWheel(int theDelta);
    bool KeyChar(SexyChar theChar);
    bool KeyDown(KeyCode key);
    bool KeyUp(KeyCode key);

    bool IsLeftButtonDown() const;
    bool IsMiddleButtonDown() const;
    bool IsRightButtonDown() const;
};
} // namespace Sexy

#endif // __WIDGETMANAGER_H__
