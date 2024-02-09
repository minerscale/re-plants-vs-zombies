#ifndef __NEWUSERDIALOG_H__
#define __NEWUSERDIALOG_H__

#include "LawnDialog.h"
#include "framework/widget/EditListener.h"
#include "widget/EditWidget.h"

class NewUserDialog : public LawnDialog, public EditListener {
public:
    LawnApp *mApp;               //+0x170
    EditWidget *mNameEditWidget; //+0x174

public:
    NewUserDialog(LawnApp *theApp, bool isRename);
    ~NewUserDialog() override;

    int GetPreferredHeight(int theWidth) override;
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void Draw(Graphics *g) override;
    void EditWidgetText(int theId, const SexyString &theString) override;
    virtual bool AllowChar(int, SexyChar theChar);
    SexyString GetName();
    void SetName(const SexyString &theName);
};

#endif
