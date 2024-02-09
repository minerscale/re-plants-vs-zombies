#ifndef __LISTWIDGET_H__
#define __LISTWIDGET_H__

#include "ScrollListener.h"
#include "Widget.h"

namespace Sexy {
using SexyStringVector = std::vector<SexyString>;
using ColorVector = std::vector<Color>;

class ScrollbarWidget;
class ListListener;
class _Font;

class ListWidget : public Widget, public ScrollListener {
public:
    enum { JUSTIFY_LEFT = 0, JUSTIFY_CENTER, JUSTIFY_RIGHT };

    enum {
        COLOR_BKG = 0,
        COLOR_OUTLINE,
        COLOR_TEXT,
        COLOR_HILITE,
        COLOR_SELECT,
        COLOR_SELECT_TEXT,
    };

public:
    int mId;
    _Font *mFont;
    ScrollbarWidget *mScrollbar;
    int mJustify;

    SexyStringVector mLines;
    ColorVector mLineColors;
    double mPosition;
    double mPageSize;
    int mHiliteIdx;
    int mSelectIdx;
    ListListener *mListListener;
    ListWidget *mParent;
    ListWidget *mChild;
    bool mSortFromChild;
    bool mDrawOutline;
    int mMaxNumericPlaces;
    int mItemHeight;

    bool mDrawSelectWhenHilited;
    bool mDoFingerWhenHilited;

    void SetHilite(int theHiliteIdx, bool notifyListener = false);

public:
    ListWidget(int theId, _Font *theFont, ListListener *theListListener);
    ~ListWidget() override;

    void RemovedFromManager(WidgetManager *theManager) override;

    virtual SexyString GetSortKey(int theIdx);
    virtual void Sort(bool ascending);
    virtual SexyString GetStringAt(int theIdx);
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    virtual int AddLine(const SexyString &theLine, bool alphabetical);
    virtual void SetLine(int theIdx, const SexyString &theString);
    virtual int GetLineCount();
    virtual int GetLineIdx(const SexyString &theLine);
    virtual void SetColor(const SexyString &theLine, const Color &theColor);
    void SetColor(int theIdx, const Color &theColor) override;
    virtual void SetLineColor(int theIdx, const Color &theColor);
    virtual void RemoveLine(int theIdx);
    virtual void RemoveAll();
    virtual int GetOptimalWidth();
    virtual int GetOptimalHeight();
    void OrderInManagerChanged() override;
    void Draw(Graphics *g) override;
    void ScrollPosition(int theId, double thePosition) override;
    void MouseMove(int x, int y) override;
    void MouseWheel(int theDelta) override;
    void MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
    void MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
    void MouseLeave() override;
    virtual void SetSelect(int theSelectIdx);
};
} // namespace Sexy

#endif // __LISTWIDGET_H__
