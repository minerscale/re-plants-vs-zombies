#ifndef __LAWNDIALOG_H__
#define __LAWNDIALOG_H__

#include "framework/widget/Dialog.h"
#include "todlib/TodCommon.h"

constexpr const int DIALOG_HEADER_OFFSET = 45;

class LawnApp;
class LawnDialog;
class Reanimation;
class LawnStoneButton;
class ReanimationWidget;

namespace Sexy {
class Checkbox;
class DialogButton;
class CheckboxListener;
} // namespace Sexy

// using namespace std;
using namespace Sexy;

class ReanimationWidget : public Widget {
public:
    LawnApp *mApp;           //+0x88
    Reanimation *mReanim;    //+0x8C
    LawnDialog *mLawnDialog; //+0x90
    float mPosX;             //+0x94
    float mPosY;             //+0x98

public:
    ReanimationWidget();
    ~ReanimationWidget() override;

    /*inline*/
    void Dispose();
    void Draw(Graphics *) override;
    void Update() override;
    void AddReanimation(float x, float y, ReanimationType theReanimationType);
};

class LawnDialog : public Dialog {
public:
    LawnApp *mApp;                   //+0x150
    int mButtonDelay;                //+0x154
    ReanimationWidget *mReanimation; //+0x158
    bool mDrawStandardBack;          //+0x15C
    LawnStoneButton *mLawnYesButton; //+0x160
    LawnStoneButton *mLawnNoButton;  //+0x164
    bool mTallBottom;                //+0x168
    bool mVerticalCenterText;        //+0x169

public:
    LawnDialog(
        LawnApp *theApp, int theId, bool isModal, const SexyString &theDialogHeader, const SexyString &theDialogLines,
        const SexyString &theDialogFooter, int theButtonMode
    );
    ~LawnDialog() override;

    int GetLeft() const;
    int GetWidth() const;
    int GetTop();
    virtual void SetButtonDelay(int theDelay);
    void Update() override;
    void ButtonPress(int theId) override;
    void ButtonDepress(int theId) override;
    virtual void CheckboxChecked();
    void KeyDown(KeyCode theKey) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    void Draw(Graphics *g) override;
    void CalcSize(int theExtraX, int theExtraY);
};

class GameOverDialog : public LawnDialog {
public:
    DialogButton *mMenuButton;

public:
    GameOverDialog(const SexyString &theMessage, bool theShowChallengeName);
    ~GameOverDialog() override;

    void ButtonDepress(int theId) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void MouseDrag(int x, int y) override;
};

#endif
