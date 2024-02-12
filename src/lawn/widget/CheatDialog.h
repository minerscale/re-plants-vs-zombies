#ifndef __CHEATDIALOG_H__
#define __CHEATDIALOG_H__

#include "LawnDialog.h"
#include "framework/widget/EditListener.h"
#include "framework/widget/EditWidget.h"

class CheatDialog : public LawnDialog, public EditListener {
public:
    LawnApp *mApp;                //+0x170
    EditWidget *mLevelEditWidget; //+0x174

public:
    CheatDialog(LawnApp *theApp);
    ~CheatDialog() override;

    int GetPreferredHeight(int theWidth) override;
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void Draw(Graphics *g) override;
    void EditWidgetText(int theId, const SexyString &theString) override;
    virtual bool AllowChar(int theId, SexyChar theChar);
    bool ApplyCheat() const;
};

#endif
