#ifndef __CONTINUEDIALOG_H__
#define __CONTINUEDIALOG_H__

#include "LawnDialog.h"

class ContinueDialog : public LawnDialog {
public:
    enum { ContinueDialog_Continue, ContinueDialog_NewGame };

public:
    DialogButton *mContinueButton; //+0x16C
    DialogButton *mNewGameButton;  //+0x170

public:
    ContinueDialog(LawnApp *theApp);
    ~ContinueDialog() override;

    int GetPreferredHeight(int theWidth) override;
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void ButtonDepress(int theId) override;
    void RestartLoopingSounds();
};

#endif
