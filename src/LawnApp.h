#ifndef __LAWNAPP_H__
#define __LAWNAPP_H__

#include "ConstEnums.h"
#include "framework/SexyApp.h"
#include "todlib/TodFoley.h"
#include <chrono>

class Board;
class GameSelector;
class ChallengeDefinition;
class SeedChooserScreen;
class AwardScreen;
class CreditScreen;
class TodFoley;
class PoolEffect;
class ZenGarden;
class PottedPlant;
class EffectSystem;
class TodParticleSystem;
class Reanimation;
class ReanimatorCache;
class ProfileMgr;
class PlayerInfo;
class Music;
class TitleScreen;
class PopDRMComm;
class ChallengeScreen;
class StoreScreen;
class AlmanacDialog;
class TypingCheck;

namespace Sexy {
class Dialog;
class Graphics;
class ButtonWidget;
}; // namespace Sexy

enum FoleyType;

using namespace Sexy;

using ButtonList = std::list<ButtonWidget *>;
using ImageList = std::list<Image *>;

class LevelStats {
public:
    int mUnusedLawnMowers;

public:
    LevelStats() { Reset(); }
    inline void Reset() { mUnusedLawnMowers = 0; }
};

class LawnApp : public SexyApp {
public:
    Board *mBoard;                                                         //+0x768
    TitleScreen *mTitleScreen;                                             //+0x76C
    GameSelector *mGameSelector;                                           //+0x770
    SeedChooserScreen *mSeedChooserScreen;                                 //+0x774
    AwardScreen *mAwardScreen;                                             //+0x778
    CreditScreen *mCreditScreen;                                           //+0x77C
    ChallengeScreen *mChallengeScreen;                                     //+0x780
    TodFoley *mSoundSystem;                                                //+0x784
    ButtonList mControlButtonList;                                         //+0x788
    ImageList mCreatedImageList;                                           //+0x794
    std::string mReferId;                                                  //+0x7A0
    std::string mRegisterLink;                                             //+0x7BC
    std::string mMod;                                                      //+0x7D8
    bool mRegisterResourcesLoaded;                                         //+0x7F4
    bool mTodCheatKeys;                                                    //+0x7F5
    GameMode mGameMode;                                                    //+0x7F8
    GameScenes mGameScene;                                                 //+0x7FC
    bool mLoadingZombiesThreadCompleted;                                   //+0x800
    bool mFirstTimeGameSelector;                                           //+0x801
    int mGamesPlayed;                                                      //+0x804
    int mMaxExecutions;                                                    //+0x808
    int mMaxPlays;                                                         //+0x80C
    int mMaxTime;                                                          //+0x810
    bool mEasyPlantingCheat;                                               //+0x814
    PoolEffect *mPoolEffect;                                               //+0x818
    ZenGarden *mZenGarden;                                                 //+0x81C
    EffectSystem *mEffectSystem;                                           //+0x820
    ReanimatorCache *mReanimatorCache;                                     //+0x824
    ProfileMgr *mProfileMgr;                                               //+0x828
    PlayerInfo *mPlayerInfo;                                               //+0x82C
    LevelStats *mLastLevelStats;                                           //+0x830
    bool mCloseRequest;                                                    //+0x834
    int mAppCounter;                                                       //+0x838
    Music *mMusic;                                                         //+0x83C
    ReanimationID mCrazyDaveReanimID;                                      //+0x840
    CrazyDaveState mCrazyDaveState;                                        //+0x844
    int mCrazyDaveBlinkCounter;                                            //+0x848
    ReanimationID mCrazyDaveBlinkReanimID;                                 //+0x84C
    int mCrazyDaveMessageIndex;                                            //+0x850
    SexyString mCrazyDaveMessageText;                                      //+0x854
    int mAppRandSeed;                                                      //+0x870
    PopDRMComm *mDRM;                                                      //+0x878
    intptr_t mSessionID;                                                   //+0x87C
    std::chrono::high_resolution_clock::duration mPlayTimeActiveSession;   //+0x880
    std::chrono::high_resolution_clock::duration mPlayTimeInactiveSession; //+0x884
    BoardResult mBoardResult;                                              //+0x888
    bool mSawYeti;                                                         //+0x88C
    TypingCheck *mKonamiCheck;                                             //+0x890
    TypingCheck *mMustacheCheck;                                           //+0x894
    TypingCheck *mMoustacheCheck;                                          //+0x898
    TypingCheck *mSuperMowerCheck;                                         //+0x89C
    TypingCheck *mSuperMowerCheck2;                                        //+0x8A0
    TypingCheck *mFutureCheck;                                             //+0x8A4
    TypingCheck *mPinataCheck;                                             //+0x8A8
    TypingCheck *mDanceCheck;                                              //+0x8AC
    TypingCheck *mDaisyCheck;                                              //+0x8B0
    TypingCheck *mSukhbirCheck;                                            //+0x8B4
    bool mMustacheMode;                                                    //+0x8B8
    bool mSuperMowerMode;                                                  //+0x8B9
    bool mFutureMode;                                                      //+0x8BA
    bool mPinataMode;                                                      //+0x8BB
    bool mDanceMode;                                                       //+0x8BC
    bool mDaisyMode;                                                       //+0x8BD
    bool mSukhbirMode;                                                     //+0x8BE
    TrialType mTrialType;                                                  //+0x8C0
    bool mDebugTrialLocked;                                                //+0x8C4
    bool mMuteSoundsForCutscene;                                           //+0x8C5

public:
    LawnApp();
    ~LawnApp() override;

    bool KillNewOptionsDialog();
    void GotFocus() override;
    void LostFocus() override;
    void InitHook() override;
    void WriteToRegistry() override;
    void ReadFromRegistry() override;
    void LoadingThreadProc() override;
    virtual void LoadingCompleted();
    void LoadingThreadCompleted() override;
    void URLOpenFailed(const std::string &theURL) override;
    void URLOpenSucceeded(const std::string &theURL) override;
    bool OpenURL(const std::string &theURL, bool shutdownOnOpen) override;
    bool DebugKeyDown(int theKey) override;
    void HandleCmdLineParam(const std::string &theParamName, const std::string &theParamValue) override;
    void ConfirmQuit();
    void ConfirmCheckForUpdates() { ; }
    void CheckForUpdates() { ; }
    void DoUserDialog();
    void FinishUserDialog(bool isYes);
    void DoCreateUserDialog();
    void DoCheatDialog();
    void FinishCheatDialog(bool isYes);
    void FinishCreateUserDialog(bool isYes);
    void DoConfirmDeleteUserDialog(const SexyString &theName);
    void FinishConfirmDeleteUserDialog(bool isYes);
    void DoRenameUserDialog(const SexyString &theName);
    void FinishRenameUserDialog(bool isYes);
    void FinishNameError(int theId);
    void FinishRestartConfirmDialog();
    void DoConfirmSellDialog(const SexyString &theMessage);
    void DoConfirmPurchaseDialog(const SexyString &theMessage);
    void FinishTimesUpDialog();
    void KillBoard();
    void MakeNewBoard();
    void StartPlaying();
    bool TryLoadGame();
    void NewGame();
    void PreNewGame(GameMode theGameMode, bool theLookForSavedGame);
    void ShowGameSelector();
    void KillGameSelector();
    void ShowAwardScreen(AwardType theAwardType, bool theShowAchievements); // @Patoke: add argument
    void KillAwardScreen();
    void ShowSeedChooserScreen();
    void KillSeedChooserScreen();
    void DoHighScoreDialog();
    void DoBackToMain();
    void DoConfirmBackToMain();
    void DoNewOptions(bool theFromGameSelector);
    void DoRegister();
    void DoRegisterError();
    bool CanDoRegisterDialog();
    /*inline*/ bool WriteCurrentUserConfig() const;
    void DoNeedRegisterDialog();
    void DoContinueDialog();
    void DoPauseDialog();
    void FinishModelessDialogs();
    Dialog *DoDialog(
        int theDialogId, bool isModal, const SexyString &theDialogHeader, const SexyString &theDialogLines,
        const SexyString &theDialogFooter, int theButtonMode
    ) override;
    virtual Dialog *DoDialogDelay(
        int theDialogId, bool isModal, const SexyString &theDialogHeader, const SexyString &theDialogLines,
        const SexyString &theDialogFooter, int theButtonMode
    );
    void Shutdown() override;
    void Init() override;
    void Start() override;
    Dialog *NewDialog(
        int theDialogId, bool isModal, const SexyString &theDialogHeader, const SexyString &theDialogLines,
        const SexyString &theDialogFooter, int theButtonMode
    ) override;
    bool KillDialog(int theDialogId) override;
    void ModalOpen() override;
    void ModalClose() override;
    void PreDisplayHook() override;
    bool ChangeDirHook(const char *theIntendedPath) override;
    virtual bool NeedRegister();
    virtual void UpdateRegisterInfo();
    void ButtonPress(int theId) override;
    void ButtonDepress(int theId) override;
    void ButtonDownTick(int theId) override;
    void ButtonMouseEnter(int theId) override;
    void ButtonMouseLeave(int theId) override;
    void ButtonMouseMove(int theId, int theX, int theY) override;
    void UpdateFrames() override;
    bool UpdateApp() override;
    /*inline*/ bool IsAdventureMode() const;
    /*inline*/ bool IsSurvivalMode() const;
    bool IsContinuousChallenge();
    /*inline*/ bool IsArtChallenge() const;
    bool NeedPauseGame() const;
    virtual void ShowResourceError(bool doExit = false);
    void ToggleSlowMo();
    void ToggleFastMo();
    void PlayFoley(FoleyType theFoleyType);
    void PlayFoleyPitch(FoleyType theFoleyType, float thePitch);
    void PlaySample(int theSoundNum) override;
    void FastLoad(GameMode theGameMode);
    static SexyString GetStageString(int theLevel);
    /*inline*/ void KillChallengeScreen();
    void ShowChallengeScreen(ChallengePage thePage);
    ChallengeDefinition &GetCurrentChallengeDef();
    void CheckForGameEnd();
    void CloseRequestAsync() override;
    /*inline*/ bool IsChallengeWithoutSeedBank();
    AlmanacDialog *
    DoAlmanacDialog(SeedType theSeedType = SeedType::SEED_NONE, ZombieType theZombieType = ZombieType::ZOMBIE_INVALID);
    bool KillAlmanacDialog();
    int GetSeedsAvailable();
    Reanimation *AddReanimation(float theX, float theY, int theRenderOrder, ReanimationType theReanimationType);
    TodParticleSystem *AddTodParticle(float theX, float theY, int theRenderOrder, ParticleEffect theEffect);
    /*inline*/ ParticleSystemID ParticleGetID(TodParticleSystem *theParticle);
    /*inline*/ TodParticleSystem *ParticleGet(ParticleSystemID theParticleID);
    /*inline*/ TodParticleSystem *ParticleTryToGet(ParticleSystemID theParticleID);
    /*inline*/ ReanimationID ReanimationGetID(Reanimation *theReanimation);
    /*inline*/ Reanimation *ReanimationGet(ReanimationID theReanimationID);
    /*inline*/ Reanimation *ReanimationTryToGet(ReanimationID theReanimationID);
    void RemoveReanimation(ReanimationID theReanimationID);
    void RemoveParticle(ParticleSystemID theParticleID);
    StoreScreen *ShowStoreScreen();
    void KillStoreScreen();
    bool HasSeedType(SeedType theSeedType);
    /*inline*/ bool SeedTypeAvailable(SeedType theSeedType);
    /*inline*/ void EndLevel();
    inline bool IsIceDemo() { return false; }
    /*inline*/ bool IsShovelLevel() const;
    /*inline*/ bool IsWallnutBowlingLevel() const;
    /*inline*/ bool IsMiniBossLevel();
    /*inline*/ bool IsSlotMachineLevel() const;
    /*inline*/ bool IsLittleTroubleLevel();
    /*inline*/ bool IsStormyNightLevel();
    /*inline*/ bool IsFinalBossLevel();
    /*inline*/ bool IsBungeeBlitzLevel();
    static /*inline*/ SeedType GetAwardSeedForLevel(int theLevel);
    SexyString GetCrazyDaveText(int theMessageIndex);
    /*inline*/ bool CanShowAlmanac();
    /*inline*/ bool IsNight();
    /*inline*/ bool CanShowStore();
    /*inline*/ bool HasBeatenChallenge(GameMode theGameMode);
    PottedPlant *GetPottedPlantByIndex(int thePottedPlantIndex);
    static /*inline*/ bool IsSurvivalNormal(GameMode theGameMode);
    static /*inline*/ bool IsSurvivalHard(GameMode theGameMode);
    static /*inline*/ bool IsSurvivalEndless(GameMode theGameMode);
    /*inline*/ bool HasFinishedAdventure() const;
    /*inline*/ bool IsFirstTimeAdventureMode();
    /*inline*/ bool CanSpawnYetis();
    void CrazyDaveEnter();
    void UpdateCrazyDave();
    void CrazyDaveTalkIndex(int theMessageIndex);
    void CrazyDaveTalkMessage(const SexyString &theMessage);
    void CrazyDaveLeave();
    void DrawCrazyDave(Graphics *g);
    void CrazyDaveDie();
    void CrazyDaveStopTalking();
    void PreloadForUser();
    int GetNumPreloadingTasks();
    int LawnMessageBox(
        int theDialogId, const SexyChar *theHeaderName, const SexyChar *theLinesName, const SexyChar *theButton1Name,
        const SexyChar *theButton2Name, int theButtonMode
    );
    //	virtual void					EnforceCursor();
    void ShowCreditScreen();
    void KillCreditScreen();
    static SexyString Pluralize(int theCount, const SexyChar *theSingular, const SexyChar *thePlural);
    int GetNumTrophies(ChallengePage thePage);
    /*inline*/ bool EarnedGoldTrophy();
    inline bool IsRegistered() { return false; }
    inline bool IsExpired() { return false; }
    inline bool IsDRMConnected() { return false; }
    /*inline*/ bool IsScaryPotterLevel();
    static /*inline*/ bool IsEndlessScaryPotter(GameMode theGameMode);
    /*inline*/ bool IsSquirrelLevel() const;
    /*inline*/ bool IsIZombieLevel() const;
    /*inline*/ bool CanShowZenGarden();
    static SexyString GetMoneyString(int theAmount);
    bool AdvanceCrazyDaveText();
    /*inline*/ bool IsWhackAZombieLevel();
    void UpdatePlayTimeStats();
    void BetaAddFile(std::list<std::string> &theUploadFileList, std::string theFileName, std::string theShortName);
    bool CanPauseNow();
    /*inline*/ bool IsPuzzleMode() const;
    /*inline*/ bool IsChallengeMode() const;
    static /*inline*/ bool IsEndlessIZombie(GameMode theGameMode);
    void CrazyDaveDoneHanding();
    inline SexyString GetCurrentLevelName() { return _S("Unknown"); }
    /*inline*/ int TrophiesNeedForGoldSunflower();
    /*inline*/ int GetCurrentChallengeIndex();
    void LoadGroup(const char *theGroupName, int theGroupAveMsToLoad);
    // void TraceLoadGroup(const char* theGroupName, int theGroupTime, int theTotalGroupWeight, int theTaskWeight);
    void CrazyDaveStopSound();
    /*inline*/ bool IsTrialStageLocked();
    /*inline*/ void FinishZenGardenToturial();
    bool UpdatePlayerProfileForFinishingLevel();
    bool SaveFileExists();
    /*inline*/ bool CanDoPinataMode();
    /*inline*/ bool CanDoDanceMode();
    /*inline*/ bool CanDoDaisyMode();
    void SwitchScreenMode(bool wantWindowed, bool is3d, bool force = false) override;
    static /*inline*/ void CenterDialog(Dialog *theDialog, int theWidth, int theHeight);
    static void HandleError(const std::string& theMsg);
};

SexyString LawnGetCurrentLevelName();
bool LawnGetCloseRequest();
bool LawnHasUsedCheatKeys();

extern bool (*gAppCloseRequest)();     //[0x69E6A0]
extern bool (*gAppHasUsedCheatKeys)(); //[0x69E6A4]
extern SexyString (*gGetCurrentLevelName)();

extern bool gIsPartnerBuild;
extern bool gFastMo;       // 0x6A9EAB
extern bool gSlowMo;       // 0x6A9EAA
extern LawnApp *gLawnApp;  // 0x6A9EC0
extern int gSlowMoCounter; // 0x6A9EC4

#endif // __LAWNAPP_H__
