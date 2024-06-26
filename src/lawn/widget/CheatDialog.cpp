#include "CheatDialog.h"
#include "ChallengeScreen.h"
#include "GameConstants.h"
#include "LawnApp.h"
#include "Resources.h"
#include "lawn/LawnCommon.h"
#include "lawn/system/PlayerInfo.h"
#include "widget/WidgetManager.h"

CheatDialog::CheatDialog(LawnApp *theApp)
    : LawnDialog(
          theApp, Dialogs::DIALOG_CHEAT, true, _S("CHEAT"), _S("Enter New Level:"), _S(""), Dialog::BUTTONS_OK_CANCEL
      ) {
    mApp = theApp;
    mVerticalCenterText = false;
    mLevelEditWidget = CreateEditWidget(0, this, this);
    mLevelEditWidget->mMaxChars = 12;
    mLevelEditWidget->AddWidthCheckFont(FONT_BRIANNETOD12, 220);

    SexyString aCheatStr;
    if (mApp->mGameMode != GameMode::GAMEMODE_ADVENTURE) {
        aCheatStr = fmt::format(_S("C{}"), (int)mApp->mGameMode);
    } else if (mApp->HasFinishedAdventure()) {
        aCheatStr = fmt::format(_S("F{}"), mApp->GetStageString(mApp->mPlayerInfo->GetLevel()).c_str());
    } else {
        aCheatStr = mApp->GetStageString(mApp->mPlayerInfo->GetLevel());
    }
    mLevelEditWidget->SetText(aCheatStr, true);

    CalcSize(110, 40);
}

CheatDialog::~CheatDialog() { delete mLevelEditWidget; }

int CheatDialog::GetPreferredHeight(const int theWidth) { return LawnDialog::GetPreferredHeight(theWidth); }

void CheatDialog::Resize(const int theX, const int theY, const int theWidth, const int theHeight) {
    LawnDialog::Resize(theX, theY, theWidth, theHeight);
    mLevelEditWidget->Resize(
        mContentInsets.mLeft + 12, mHeight - 155, mWidth - mContentInsets.mLeft - mContentInsets.mRight - 24, 28
    );
}

void CheatDialog::AddedToManager(WidgetManager *theWidgetManager) {
    LawnDialog::AddedToManager(theWidgetManager);
    AddWidget(mLevelEditWidget);
    theWidgetManager->SetFocus(mLevelEditWidget);
}

void CheatDialog::RemovedFromManager(WidgetManager *theWidgetManager) {
    LawnDialog::RemovedFromManager(theWidgetManager);
    RemoveWidget(mLevelEditWidget);
}

void CheatDialog::Draw(Graphics *g) {
    LawnDialog::Draw(g);
    DrawEditBox(g, mLevelEditWidget);
}

void CheatDialog::EditWidgetText(const int theId, const SexyString &theString) {
    (void)theId;
    (void)theString;
    mApp->ButtonDepress(mId + 2000);
}

bool CheatDialog::AllowChar(const int theId, const SexyChar theChar) {
    (void)theId;
    return isdigit(theChar) || theChar == _S('-') || theChar == _S('c') || theChar == _S('C') || theChar == _S('f') ||
           theChar == _S('F');
}

bool CheatDialog::ApplyCheat() const {
    int aChallengeIndex;
    if (sscanf(mLevelEditWidget->mString.c_str(), _S("c%d"), &aChallengeIndex) == 1 ||
        sscanf(mLevelEditWidget->mString.c_str(), _S("C%d"), &aChallengeIndex) == 1) {
        mApp->mGameMode = static_cast<GameMode>(ClampInt(aChallengeIndex, 0, NUM_CHALLENGE_MODES));
        return true;
    }

    int aLevel = -1;
    int aFinishedAdventure = 0;
    int aArea, aSubArea;
    if (sscanf(mLevelEditWidget->mString.c_str(), _S("f%d-%d"), &aArea, &aSubArea) == 2 ||
        sscanf(mLevelEditWidget->mString.c_str(), _S("F%d-%d"), &aArea, &aSubArea) == 2) {
        aLevel = (aArea - 1) * LEVELS_PER_AREA + aSubArea;
        aFinishedAdventure = 1;
    } else if (sscanf(mLevelEditWidget->mString.c_str(), _S("f%d"), &aLevel) == 1 || sscanf(mLevelEditWidget->mString.c_str(), _S("F%d"), &aLevel) == 1) {
        aFinishedAdventure = 1;
    } else if (sscanf(mLevelEditWidget->mString.c_str(), _S("%d-%d"), &aArea, &aSubArea) == 2) {
        aLevel = (aArea - 1) * LEVELS_PER_AREA + aSubArea;
    } else {
        sscanf(mLevelEditWidget->mString.c_str(), _S("%d"), &aLevel);
    }

    if (aLevel <= 0) {
        mApp->DoDialog(
            Dialogs::DIALOG_CHEATERROR, true, _S("Enter Level"),
            _S("Invalid Level. Do 'number' or 'area-subarea' or 'Cnumber' or 'Farea-subarea'."), _S("OK"),
            Dialog::BUTTONS_FOOTER
        );
        return false;
    }

    mApp->mGameMode = GameMode::GAMEMODE_ADVENTURE;
    mApp->mPlayerInfo->SetLevel(aLevel);
    mApp->mPlayerInfo->mFinishedAdventure = aFinishedAdventure;
    mApp->WriteCurrentUserConfig();
    return true;
}
