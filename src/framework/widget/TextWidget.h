#ifndef __TEXTWIDGET_H__
#define __TEXTWIDGET_H__

#include "ScrollListener.h"
#include "Widget.h"

namespace Sexy {
class ScrollbarWidget;
class _Font;

using SexyStringVector = std::vector<SexyString>;
using IntVector = std::vector<int>;

class TextWidget : public Widget, public ScrollListener {
public:
    _Font *mFont;
    ScrollbarWidget *mScrollbar;

    SexyStringVector mLogicalLines;
    SexyStringVector mPhysicalLines;
    IntVector mLineMap;
    double mPosition;
    double mPageSize;
    bool mStickToBottom;
    int mHiliteArea[2][2];
    int mMaxLines;

public:
    TextWidget();

    virtual SexyStringVector GetLines();
    virtual void SetLines(SexyStringVector theNewLines);
    virtual void Clear();
    virtual void DrawColorString(Graphics *g, const SexyString &theString, int x, int y, bool useColors);
    virtual void
    DrawColorStringHilited(Graphics *g, const SexyString &theString, int x, int y, int theStartPos, int theEndPos);
    virtual int GetStringIndex(const SexyString &theString, int thePixel);

    virtual int GetColorStringWidth(const SexyString &theString);
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    virtual Color GetLastColor(const SexyString &theString);
    virtual void AddToPhysicalLines(int theIdx, const SexyString &theLine);

    virtual void AddLine(const SexyString &theString);
    virtual bool SelectionReversed();
    virtual void GetSelectedIndices(int theLineIdx, int *theIndices);
    void Draw(Graphics *g) override;
    void ScrollPosition(int theId, double thePosition) override;
    virtual void GetTextIndexAt(int x, int y, int *thePosArray);
    virtual SexyString GetSelection();

    void MouseDown(int x, int y, int theClickCount) override;
    void MouseDrag(int x, int y) override;

    void KeyDown(KeyCode theKey) override;
};
} // namespace Sexy

#endif //__TEXTWIDGET_H__
