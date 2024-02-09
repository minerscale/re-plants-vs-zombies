#ifndef __DIALOG_H__
#define __DIALOG_H__

#include "ButtonListener.h"
#include "Widget.h"

namespace Sexy {
class DialogListener;
class ButtonWidget;
class DialogButton;
class _Font;

extern SexyString DIALOG_YES_STRING;
extern SexyString DIALOG_NO_STRING;
extern SexyString DIALOG_OK_STRING;
extern SexyString DIALOG_CANCEL_STRING;

using StringVector = std::vector<std::string>;

class Dialog : public Widget, public ButtonListener {
public:
    enum { BUTTONS_NONE, BUTTONS_YES_NO, BUTTONS_OK_CANCEL, BUTTONS_FOOTER };

    enum { ID_YES = 1000, ID_NO = 1001, ID_OK = 1000, ID_CANCEL = 1001, ID_FOOTER = 1000 };

    enum {
        COLOR_HEADER = 0,
        COLOR_LINES,
        COLOR_FOOTER,
        COLOR_BUTTON_TEXT,
        COLOR_BUTTON_TEXT_HILITE,
        COLOR_BKG,
        COLOR_OUTLINE,
        NUM_COLORS
    };

    DialogListener *mDialogListener;
    Image *mComponentImage;
    DialogButton *mYesButton;
    DialogButton *mNoButton;
    int mNumButtons;

    SexyString mDialogHeader;
    SexyString mDialogFooter;
    SexyString mDialogLines;

    int mButtonMode;
    _Font *mHeaderFont;
    _Font *mLinesFont;
    int mTextAlign;
    int mLineSpacingOffset;
    int mButtonHeight;
    Insets mBackgroundInsets;
    Insets mContentInsets;
    int mSpaceAfterHeader;
    bool mDragging;
    int mDragMouseX;
    int mDragMouseY;
    int mId;
    bool mIsModal;
    int mResult;

    int mButtonHorzSpacing;
    int mButtonSidePadding;

public:
    void EnsureFonts();

public:
    Dialog(
        Image *theComponentImage, Image *theButtonComponentImage, int theId, bool isModal,
        const SexyString &theDialogHeader, const SexyString &theDialogLines, const SexyString &theDialogFooter,
        int theButtonMode
    ); // UNICODE

    ~Dialog() override;

    virtual void SetButtonFont(_Font *theFont);
    virtual void SetHeaderFont(_Font *theFont);
    virtual void SetLinesFont(_Font *theFont);

    void SetColor(int theIdx, const Color &theColor) override;
    virtual int GetPreferredHeight(int theWidth);

    void Draw(Graphics *g) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void OrderInManagerChanged() override;
    void Resize(int theX, int theY, int theWidth, int theHeight) override;

    void MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
    void MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
    void MouseDrag(int x, int y) override;
    void MouseUp(int x, int y) override { Widget::MouseUp(x, y); }
    void MouseUp(int x, int y, int theClickCount) override { Widget::MouseUp(x, y, theClickCount); }
    void MouseUp(int x, int y, int theBtnNum, int theClickCount) override;
    void Update() override;
    virtual bool IsModal();
    virtual int WaitForResult(bool autoKill = true);

    void ButtonPress(int theId) override;
    void ButtonDepress(int theId) override;

    void ButtonDownTick(int) override {}

    void ButtonMouseEnter(int) override {}

    void ButtonMouseLeave(int) override {}

    void ButtonMouseMove(int, int, int) override {}
};
} // namespace Sexy

#endif //__DIALOG_H__
