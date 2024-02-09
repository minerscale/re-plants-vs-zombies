#ifndef __IMITATERDIALOG_H__
#define __IMITATERDIALOG_H__

#include "LawnDialog.h"

class ToolTipWidget;

class ImitaterDialog : public LawnDialog {
public:
    ToolTipWidget *mToolTip;
    SeedType mToolTipSeed;

public:
    ImitaterDialog();
    ~ImitaterDialog() override;

    SeedType SeedHitTest(int x, int y);
    void UpdateCursor();
    void Update() override;
    /*inline*/
    void GetSeedPosition(int theIndex, int &x, int &y);
    void Draw(Graphics *g) override;
    void ShowToolTip();
    /*inline*/
    void RemoveToolTip();
    void MouseDown(int x, int y, int theClickCount) override;

    void MouseUp(int, int, int) override {}
};

#endif
