#ifndef __USERDIALOG_H__
#define __USERDIALOG_H__

#include "LawnDialog.h"
#include "framework/widget/EditListener.h"
#include "framework/widget/ListListener.h"

namespace Sexy {
class ListWidget;
};

class UserDialog : public LawnDialog, public ListListener, public EditListener {
protected:
    enum { UserDialog_RenameUser, UserDialog_DeleteUser };

public:
    ListWidget *mUserList;       //+0x174
    DialogButton *mRenameButton; //+0x178
    DialogButton *mDeleteButton; //+0x17C
    int mNumUsers;               //+0x180

public:
    UserDialog(LawnApp *theApp);
    ~UserDialog() override;

    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    int GetPreferredHeight(int theWidth) override;
    void AddedToManager(WidgetManager *theWidgetManager) override;
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void ListClicked(int theId, int theIdx, int theClickCount) override;

    void ListClosed(int) override {}

    void ListHiliteChanged(int, int, int) override {}

    void ButtonDepress(int theId) override;
    void EditWidgetText(int theId, const SexyString &theString) override;
    virtual bool AllowChar(int theId, SexyChar theChar);
    void Draw(Graphics *g) override;
    void FinishDeleteUser();
    void FinishRenameUser(const SexyString &theNewName) const;
    SexyString GetSelName() const;
};

#endif
