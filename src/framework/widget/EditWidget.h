#ifndef __EDITWIDGET_H__
#define __EDITWIDGET_H__

#include "Widget.h"

namespace Sexy {
class _Font;
class EditListener;

class EditWidget : public Widget {
public:
    enum { COLOR_BKG, COLOR_OUTLINE, COLOR_TEXT, COLOR_HILITE, COLOR_HILITE_TEXT, NUM_COLORS };

    int mId;
    SexyString mString;
    SexyString mPasswordDisplayString;
    _Font *mFont;

    struct WidthCheck {
        _Font *mFont;
        int mWidth;
    };

    using WidthCheckList = std::list<WidthCheck>;
    WidthCheckList mWidthCheckList;

    EditListener *mEditListener;
    bool mShowingCursor;
    bool mDrawSelOverride; // set this to true to draw selected text even when not in focus
    bool mHadDoubleClick;
    // Used to fix a bug with double clicking to hilite a word after the widget manager started calling mouse drag
    // before mouse down/up events
    int mCursorPos;
    int mHilitePos;
    int mBlinkAcc;
    int mBlinkDelay;
    int mLeftPos;
    int mMaxChars;
    int mMaxPixels;
    SexyChar mPasswordChar;

    SexyString mUndoString;
    int mUndoCursor;
    int mUndoHilitePos;
    int mLastModifyIdx;

protected:
    virtual void ProcessKey(KeyCode theKey, SexyChar theChar);
    SexyString &GetDisplayString();
    virtual void HiliteWord();
    void UpdateCaretPos();

public:
    virtual void SetFont(_Font *theFont, _Font *theWidthCheckFont = NULL);
    virtual void SetText(const SexyString &theText, bool leftPosToZero = true);
    virtual bool IsPartOfWord(SexyChar theChar);
    virtual int GetCharAt(int x, int y);

    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    void Draw(Graphics *g) override; // Already translated;

    void Update() override;
    void MarkDirty() override;

    bool WantsFocus() override;
    void GotFocus() override;
    void LostFocus() override;
    virtual void FocusCursor(bool bigJump);

    void KeyDown(KeyCode theKey) override;
    void KeyChar(SexyChar theChar) override;

    void MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
    void MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
    void MouseUp(int x, int y) override { Widget::MouseUp(x, y); }
    void MouseUp(int x, int y, int theClickCount) override { Widget::MouseUp(x, y, theClickCount); }
    void MouseUp(int x, int y, int theBtnNum, int theClickCount) override;
    void MouseDrag(int x, int y) override;
    void MouseEnter() override;
    void MouseLeave() override;
    void ClearWidthCheckFonts();
    void AddWidthCheckFont(_Font *theFont, int theMaxPixels = -1); // defaults to mMaxPixels
    void EnforceMaxPixels();

public:
    EditWidget(int theId, EditListener *theEditListener);
    ~EditWidget() override;
};
} // namespace Sexy

#endif //__EDITWIDGET_H__
