#include "BoardInclude.h"
#include "Common.h"
#include "ConstEnums.h"
#include "ZenGarden.h"
#include "lawn/LawnCommon.h"
#include "misc/MTRand.h"
#include "sound/SoundInstance.h"
#include "sound/SoundManager.h"
#include "system/Music.h"
#include "system/PlayerInfo.h"
#include "system/PoolEffect.h"
#include "system/SaveGame.h"
#include "system/TypingCheck.h"
#include "todlib/Attachment.h"
#include "todlib/EffectSystem.h"
#include "todlib/Reanimator.h"
#include "todlib/TodCommon.h"
#include "todlib/TodDebug.h"
#include "todlib/TodFoley.h"
#include "todlib/TodParticle.h"
#include "todlib/TodStringFile.h"
#include "todlib/Trail.h"
#include "widget/ChallengeScreen.h"
#include "widget/Dialog.h"
#include "widget/LawnDialog.h"
#include "widget/SeedChooserScreen.h"
#include "widget/StoreScreen.h"
#include "widget/WidgetManager.h"
#include <chrono>

// #define SEXY_PERF_ENABLED
#include "misc/PerfTimer.h"
#include "widget/AchievementsScreen.h"

bool gShownMoreSunTutorial = false;

// 0x407B50
//  GOTY @Patoke: 0x40A3C0
Board::Board(LawnApp *theApp) {
    mApp = theApp;
    mApp->mBoard = this;
    TodHesitationTrace("preboard");

    mZombies.DataArrayInitialize(1024U, "zombies");
    mPlants.DataArrayInitialize(1024U, "plants");
    mProjectiles.DataArrayInitialize(1024U, "projectiles");
    mCoins.DataArrayInitialize(1024U, "coins");
    mLawnMowers.DataArrayInitialize(32U, "lawnmowers");
    mGridItems.DataArrayInitialize(128U, "griditems");
    TodHesitationTrace("board dataarrays");

    mApp->mEffectSystem->EffectSystemFreeAll();
    mBoardData.mBoardRandSeed = mApp->mAppRandSeed;
    if (mApp->IsSurvivalMode()) {
        mBoardData.mBoardRandSeed = Rand();
    }
    mBoardData.mCoinBankFadeCount = 0;
    mBoardData.mLevel = 0;
    mCursorObject = new CursorObject();
    mCursorPreview = new CursorPreview();
    mSeedBank = new SeedBank();
    mCutScene = new CutScene();
    mBoardData.mSpecialGraveStoneX = -1;
    mBoardData.mSpecialGraveStoneY = -1;
    for (int i = 0; i < MAX_GRID_SIZE_X; i++) {
        for (int j = 0; j < MAX_GRID_SIZE_Y; j++) {
            mBoardData.mGridSquareType[i][j] = GridSquareType::GRIDSQUARE_GRASS;
            mBoardData.mGridCelLook[i][j] = Rand(20);
            mBoardData.mGridCelOffset[i][j][0] = Rand(10) - 5;
            mBoardData.mGridCelOffset[i][j][1] = Rand(10) - 5;
        }

        for (int k = 0; k < MAX_GRID_SIZE_Y + 1; k++) {
            mBoardData.mGridCelFog[i][k] = 0;
        }
    }
    mBoardData.mFogOffset = 0.0f;
    mBoardData.mSunCountDown = 0;
    mBoardData.mShakeCounter = 0;
    mBoardData.mShakeAmountX = 0;
    mBoardData.mShakeAmountY = 0;
    mBoardData.mPaused = false;
    mBoardData.mLevelAwardSpawned = false;
    mBoardData.mFlagRaiseCounter = 0;
    mBoardData.mIceTrapCounter = 0;
    mBoardData.mLevelComplete = false;
    mBoardData.mBoardFadeOutCounter = -1;
    mBoardData.mNextSurvivalStageCounter = 0;
    mBoardData.mScoreNextMowerCounter = 0;
    mBoardData.mProgressMeterWidth = 0;
    mBoardData.mPoolSparklyParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
    mBoardData.mFogBlownCountDown = 0;
    mBoardData.mFwooshCountDown = 0;
    mBoardData.mTimeStopCounter = 0;
    mBoardData.mCobCannonCursorDelayCounter = 0;
    mBoardData.mCobCannonMouseX = 0;
    mBoardData.mCobCannonMouseY = 0;
    mBoardData.mDroppedFirstCoin = false;
    mBoardData.mBonusLawnMowersRemaining = 0;
    mBoardData.mEnableGraveStones = false;
    mBoardData.mHelpIndex = AdviceType::ADVICE_NONE;
    mBoardData.mEffectCounter = 0;
    mBoardData.mDrawCount = 0;
    mBoardData.mRiseFromGraveCounter = 0;
    mBoardData.mFinalWaveSoundCounter = 0;
    mBoardData.mKilledYeti = false;
    mBoardData.mTriggeredLawnMowers = 0;
    mBoardData.mPlayTimeActiveLevel = std::chrono::milliseconds(0);
    mBoardData.mPlayTimeInactiveLevel = std::chrono::milliseconds(0);
    mBoardData.mMaxSunPlants = 0;
    mBoardData.mStartDrawTime = 0;
    mBoardData.mIntervalDrawTime = 0;
    mBoardData.mIntervalDrawCountStart = 0;
    mBoardData.mPreloadTime = std::chrono::milliseconds(0);
    mBoardData.mGameID = static_cast<time_t>(0);
    mBoardData.mMinFPS = 1000.0f;
    mBoardData.mGravesCleared = 0;
    mBoardData.mPlantsEaten = 0;
    mBoardData.mPlantsShoveled = 0;
    mBoardData.mPeaShooterUsed = false;            // @Patoke: added construct
    mBoardData.mCatapultPlantsUsed = false;        // @Patoke: added construct
    mBoardData.mMushroomAndCoffeeBeansOnly = true; // @Patoke: added construct
    mBoardData.mMushroomsUsed = false;             // @Patoke: added construct
    mBoardData.mLevelCoinsCollected = 0;
    mBoardData.mCoinsCollected = 0;
    mBoardData.mDiamondsCollected = 0;
    mBoardData.mPottedPlantsCollected = 0;
    mBoardData.mChocolateCollected = 0;
    for (auto &y : mBoardData.mFwooshID) {
        for (auto &x : y) {
            x = ReanimationID::REANIMATIONID_NULL;
        }
    }
    mBoardData.mPrevMouseX = -1;
    mBoardData.mPrevMouseY = -1;
    mBoardData.mFinalBossKilled = false;
    mBoardData.mMustacheMode = mApp->mMustacheMode;
    mBoardData.mSuperMowerMode = mApp->mSuperMowerMode;
    mBoardData.mFutureMode = mApp->mFutureMode;
    mBoardData.mPinataMode = mApp->mPinataMode;
    mBoardData.mDanceMode = mApp->mDanceMode;
    mBoardData.mDaisyMode = mApp->mDaisyMode;
    mBoardData.mSukhbirMode = mApp->mSukhbirMode;
    mBoardData.mShowShovel = false;
    mToolTip = new ToolTipWidget();
    // mDebugFont = new SysFont("Arial Unicode MS", 10, true, false, false);
    mDebugFont = nullptr;
    mAdvice = new MessageWidget(mApp);
    mBoardData.mBackground = BackgroundType::BACKGROUND_1_DAY;
    mBoardData.mMainCounter = 0;
    mBoardData.mTutorialState = TutorialState::TUTORIAL_OFF;
    mBoardData.mTutorialTimer = -1;
    mBoardData.mTutorialParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
    mChallenge = new Challenge();
    mClip = false;
    mBoardData.mDebugTextMode = DebugTextMode::DEBUG_TEXT_NONE;
    mMenuButton = new GameButton(0);
    mMenuButton->mDrawStoneButton = true;
    mStoreButton = nullptr;
    mIgnoreMouseUp = false;

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        mMenuButton->SetLabel(_S("[MAIN_MENU_BUTTON]"));
        mMenuButton->Resize(628, -10, 163, 46);

        mStoreButton = new GameButton(1);
        mStoreButton->mButtonImage = IMAGE_ZENSHOPBUTTON;
        mStoreButton->mOverImage = IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
        mStoreButton->mDownImage = IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
        mStoreButton->mParentWidget = this;
        mStoreButton->Resize(678, 33, IMAGE_ZENSHOPBUTTON->mWidth, 40);
    } else {
        mMenuButton->SetLabel(_S("[MENU_BUTTON]"));
        mMenuButton->Resize(681, -10, 117, 46);
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
        mStoreButton = new GameButton(1);
        mStoreButton->mDrawStoneButton = true;
        mStoreButton->mBtnNoDraw = true;
        mStoreButton->mDisabled = true;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL) {
        mMenuButton->SetLabel(_S("[MAIN_MENU_BUTTON]"));
        mMenuButton->Resize(628, -10, 163, 46);

        mStoreButton = new GameButton(1);
        mStoreButton->mDrawStoneButton = true;
        mStoreButton->mBtnNoDraw = true;
        mStoreButton->SetLabel(_S("[GET_FULL_VERSION_BUTTON]"));
    }
}

// 0x408670、0x408690
Board::~Board() {
    delete mAdvice;
    delete mCursorObject;
    delete mCursorPreview;
    delete mSeedBank;
    delete mMenuButton;
    delete mStoreButton;

    mZombies.DataArrayDispose();
    mPlants.DataArrayDispose();
    mProjectiles.DataArrayDispose();
    mCoins.DataArrayDispose();
    mLawnMowers.DataArrayDispose();
    mGridItems.DataArrayDispose();

    delete mToolTip;
    /*
    if (mDebugFont)
    {
        delete mDebugFont;
    }*/
    delete mCutScene;
    delete mChallenge;
}

void BoardInitForPlayer() { gShownMoreSunTutorial = false; }

// 0x408A70
//  GOTY @Patoke: 0x40B320
void Board::DisposeBoard() const {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) mApp->mZenGarden->LeaveGarden();
    if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) mChallenge->TreeOfWisdomLeave();

    mApp->mSoundSystem->StopFoley(FoleyType::FOLEY_RAIN);
    mApp->mZenGarden->mBoard = nullptr;
    mApp->CrazyDaveDie();
    mApp->mEffectSystem->EffectSystemFreeAll();
}

// 0x408B00
bool Board::AreEnemyZombiesOnScreen() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && !aZombie->mMindControlled) {
            return true;
        }
    }
    return false;
}

// 0x408B60
//  GOTY @Patoke: 0x40B4A0
int Board::CountZombiesOnScreen() {
    int aCount = 0;
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && !aZombie->mMindControlled && aZombie->IsOnBoard()) {
            aCount++;
        }
    }
    return aCount;
}

// GOTY @Patoke: 0x40B3B0
int Board::GetLiveGargantuarCount() {
    int aCount = 0;
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mDead && aZombie->mHasHead && !aZombie->IsDeadOrDying() && aZombie->IsOnBoard() &&
            (aZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR ||
             aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR)) {
            aCount++;
        }
    }
    return aCount;
}

// 0x408BF0
int Board::CountUntriggerLawnMowers() {
    int aCount = 0;
    LawnMower *aLawnMower = nullptr;
    while (IterateLawnMowers(aLawnMower)) {
        if (aLawnMower->mMowerState != LawnMowerState::MOWER_TRIGGERED &&
            aLawnMower->mMowerState != LawnMowerState::MOWER_SQUISHED) {
            aCount++;
        }
    }
    return aCount;
}

// 0x408C30
void Board::TryToSaveGame() {
    const std::string aFileName = GetSavedGameName(mApp->mGameMode, mApp->mPlayerInfo->mId);

    if (NeedSaveGame()) {
        if (mBoardData.mBoardFadeOutCounter > 0) {
            CompleteEndLevelSequenceForSaving();
            return;
        }

        MkDir(GetAppDataFolder() + "userdata");
        mApp->mMusic->GameMusicPause(true);
        LawnSaveGame(this, aFileName);
        // mApp->ClearUpdateBacklog();
        SurvivalSaveScore();
    }
}

// 0x408DA0
bool Board::NeedSaveGame() const {
    return mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ICE && mApp->mGameMode != GameMode::GAMEMODE_UPSELL &&
           mApp->mGameMode != GameMode::GAMEMODE_INTRO && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
           mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM && mApp->mGameScene == GameScenes::SCENE_PLAYING;
}

void Board::SaveGame(const std::string &theFileName) { LawnSaveGame(this, theFileName); }

// GOTY @Patoke: 0x40B739
void Board::ResetFPSStats() {
    const clock_t aTickCount = clock();
    mBoardData.mStartDrawTime = aTickCount;
    mBoardData.mIntervalDrawTime = aTickCount;
    mBoardData.mDrawCount = 1;
    mBoardData.mIntervalDrawCountStart = 1;
}

// 0x408DE0
//  GOTY @Patoke: 0x40B710
bool Board::LoadGame(const std::string &theFileName) {
    if (!LawnLoadGame(this, theFileName)) return false;

    LoadBackgroundImages();
    // mApp->ClearUpdateBacklog();
    ResetFPSStats();
    UpdateLayers();
    return true;
}

// 0x408E40
GridItem *Board::GetGridItemAt(const GridItemType theGridItemType, const int theGridX, const int theGridY) {
    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY &&
            aGridItem->mGridItemType == theGridItemType) {
            return aGridItem;
        }
    }
    return nullptr;
}

// 0x408E90
GridItem *Board::GetRake() {
    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridItemType == GridItemType::GRIDITEM_RAKE) {
            return aGridItem;
        }
    }
    return nullptr;
}

GridItem *Board::GetCraterAt(const int theGridX, const int theGridY) {
    return GetGridItemAt(GridItemType::GRIDITEM_CRATER, theGridX, theGridY);
}

GridItem *Board::GetGraveStoneAt(const int theGridX, const int theGridY) {
    return GetGridItemAt(GridItemType::GRIDITEM_GRAVESTONE, theGridX, theGridY);
}

GridItem *Board::GetLadderAt(const int theGridX, const int theGridY) {
    return GetGridItemAt(GridItemType::GRIDITEM_LADDER, theGridX, theGridY);
}

GridItem *Board::GetScaryPotAt(const int theGridX, const int theGridY) {
    return GetGridItemAt(GridItemType::GRIDITEM_SCARY_POT, theGridX, theGridY);
}

/*
GridItem* Board::GetSquirrelAt(int theGridX, int theGridY)
{
    return GetGridItemAt(GridItemType::GRIDITEM_SQUIRREL, theGridX, theGridY);
}
*/

GridItem *Board::GetZenToolAt(const int theGridX, const int theGridY) {
    return GetGridItemAt(GridItemType::GRIDITEM_ZEN_TOOL, theGridX, theGridY);
}

// 0x408ED0
bool Board::CanAddGraveStoneAt(const int theGridX, const int theGridY) {
    if (mBoardData.mGridSquareType[theGridX][theGridY] != GridSquareType::GRIDSQUARE_GRASS &&
        mBoardData.mGridSquareType[theGridX][theGridY] != GridSquareType::GRIDSQUARE_HIGH_GROUND) {
        return false;
    }

    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY) {
            if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE ||
                aGridItem->mGridItemType == GridItemType::GRIDITEM_CRATER ||
                aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER)
                return false;
        }
    }
    return true;
}

int Board::MakeRenderOrder(const RenderLayer theRenderLayer, const int theRow, const int theLayerOffset) {
    return theRow * static_cast<int>(RenderLayer::RENDER_LAYER_ROW_OFFSET) + theRenderLayer + theLayerOffset;
}

// 0x408F40
GridItem *Board::AddALadder(const int theGridX, const int theGridY) {
    GridItem *aLadder = mGridItems.DataArrayAlloc();
    aLadder->mGridItemType = GridItemType::GRIDITEM_LADDER;
    aLadder->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_PLANT, theGridY, 800);
    aLadder->mGridX = theGridX;
    aLadder->mGridY = theGridY;
    return aLadder;
}

// 0x408F80
GridItem *Board::AddACrater(const int theGridX, const int theGridY) {
    GridItem *aCrater = mGridItems.DataArrayAlloc();
    aCrater->mGridItemType = GridItemType::GRIDITEM_CRATER;
    aCrater->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, theGridY, 1);
    aCrater->mGridX = theGridX;
    aCrater->mGridY = theGridY;
    return aCrater;
}

GridItem *Board::AddAGraveStone(const int theGridX, const int theGridY) {
    GridItem *aGraveStone = mGridItems.DataArrayAlloc();
    aGraveStone->mGridItemType = GridItemType::GRIDITEM_GRAVESTONE;
    aGraveStone->mGridItemCounter = -Rand(50);
    aGraveStone->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, theGridY, 3);
    aGraveStone->mGridX = theGridX;
    aGraveStone->mGridY = theGridY;
    return aGraveStone;
}

// 0x408FC0
void Board::AddGraveStones(const int theGridX, int theCount, MTRand &theLevelRNG) {
    TOD_ASSERT(theCount <= MAX_GRID_SIZE_Y);

    // 这里姑且加一个原版没有的、对于本列能否生成墓碑的判断
    // 如果没有这个判断，当本列不存在足够多的格子可以放置墓碑时，游戏会卡死
    // GridItem* aGridItem = nullptr;
    // bool aAllowGraveStone[MAX_GRID_SIZE_Y] = { false };
    int aGridAllowGraveStonesCount = 0;
    for (int y = 0; y < MAX_GRID_SIZE_Y; y++) {
        if (CanAddGraveStoneAt(theGridX, y)) {
            aGridAllowGraveStonesCount++;
        }
    }
    theCount = std::min(theCount, aGridAllowGraveStonesCount);

    int i = 0;
    while (i < theCount) {
        const int aGridY = theLevelRNG.Next(static_cast<unsigned long>(MAX_GRID_SIZE_Y));
        // if (aAllowGraveStone[aGridY])
        //{
        //	aAllowGraveStone[aGridY] = false;
        //	GridItem* aGraveStone = AddAGraveStone(theGridX, aGridY);
        //	++i;
        // }
        //  上述写法虽然效率更高，但当 AddAGraveStone() 函数被修改后，不能保证 aAllowGraveStone 仍然有效
        //  故这里仍然采用如下的原版的写法，仅在上面对 theCount 进行修正
        if (CanAddGraveStoneAt(theGridX, aGridY)) {
            const GridItem *aGraveStone = AddAGraveStone(theGridX, aGridY);
            (void)aGraveStone; // unused
            ++i;
        }
    }
}

// 0x409050
int Board::GetNumWavesPerFlag() const {
    return (mApp->IsFirstTimeAdventureMode() && mBoardData.mNumWaves < 10) ? mBoardData.mNumWaves : 10;
}

// 0x409080
bool Board::IsFlagWave(const int theWaveNumber) {
    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 1) return false;

    const int aWavesPerFlag = GetNumWavesPerFlag();
    return theWaveNumber % aWavesPerFlag == aWavesPerFlag - 1;
}

// 0x4090F0
void ZombiePickerInitForWave(ZombiePicker *theZombiePicker) { memset(theZombiePicker, 0, sizeof(ZombiePicker)); }

// 0x409170
void ZombiePickerInit(ZombiePicker *theZombiePicker) {
    ZombiePickerInitForWave(theZombiePicker);
    memset(theZombiePicker->mAllWavesZombieTypeCount, 0, sizeof(theZombiePicker->mAllWavesZombieTypeCount));
}

// 0x409240
void Board::PutZombieInWave(const ZombieType theZombieType, const int theWaveNumber, ZombiePicker *theZombiePicker) {
    TOD_ASSERT(theWaveNumber < MAX_ZOMBIE_WAVES && theZombiePicker->mZombieCount < MAX_ZOMBIES_IN_WAVE);
    mBoardData.mZombiesInWave[theWaveNumber][theZombiePicker->mZombieCount++] = theZombieType;
    if (theZombiePicker->mZombieCount < MAX_ZOMBIES_IN_WAVE) {
        mBoardData.mZombiesInWave[theWaveNumber][theZombiePicker->mZombieCount] = ZombieType::ZOMBIE_INVALID;
    }
    theZombiePicker->mZombiePoints -= GetZombieDefinition(theZombieType).mZombieValue;
    theZombiePicker->mZombieTypeCount[theZombieType]++;
    theZombiePicker->mAllWavesZombieTypeCount[theZombieType]++;
}

// 0x409290
void Board::PutInMissingZombies(const int theWaveNumber, ZombiePicker *theZombiePicker) {
    for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES;
         aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1)) {
        if (theZombiePicker->mZombieTypeCount[static_cast<int>(aZombieType)] <= 0 &&
            aZombieType != ZombieType::ZOMBIE_YETI && CanZombieSpawnOnLevel(aZombieType, mBoardData.mLevel)) {
            PutZombieInWave(aZombieType, theWaveNumber, theZombiePicker);
        }
    }
}

// 0x4092E0
void Board::PickZombieWaves() {
    // ====================================================================================================
    // ▲ 设定关卡总波数
    // ====================================================================================================
    if (mApp->IsAdventureMode()) {
        if (mApp->IsWhackAZombieLevel()) {
            mBoardData.mNumWaves = 8;
        } else {
            mBoardData.mNumWaves = gZombieWaves[ClampInt(mBoardData.mLevel - 1, 0, 49)];
            if (!mApp->IsFirstTimeAdventureMode() && !mApp->IsMiniBossLevel()) {
                mBoardData.mNumWaves = mBoardData.mNumWaves < 10 ? 20 : mBoardData.mNumWaves + 10;
            }
        }
    } else {
        const GameMode aGameMode = mApp->mGameMode;
        if (mApp->IsSurvivalMode() || aGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
            mBoardData.mNumWaves = GetNumWavesPerSurvivalStage();
        else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || aGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || mApp->IsSquirrelLevel())
            mBoardData.mNumWaves = 0;
        else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE) mBoardData.mNumWaves = 12;
        else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING || aGameMode == GameMode::GAMEMODE_CHALLENGE_AIR_RAID || aGameMode == GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER || aGameMode == GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY || aGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT || aGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS || aGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL)
            mBoardData.mNumWaves = 20;
        else if (mApp->IsStormyNightLevel() || mApp->IsLittleTroubleLevel() || mApp->IsBungeeBlitzLevel() || aGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN || mApp->IsShovelLevel() || aGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2 || aGameMode == GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2 || aGameMode == GameMode::GAMEMODE_CHALLENGE_POGO_PARTY)
            mBoardData.mNumWaves = 30;
        else mBoardData.mNumWaves = 40;
    }

    // ====================================================================================================
    // ▲ 一些准备工作
    // ====================================================================================================
    ZombiePicker aZombiePicker;
    ZombiePickerInit(&aZombiePicker);
    const ZombieType aIntroZombieType = GetIntroducedZombieType();
    TOD_ASSERT(mBoardData.mNumWaves <= MAX_ZOMBIE_WAVES);

    // ====================================================================================================
    // ▲ 遍历每一波并填充每波的出怪列表
    // ====================================================================================================
    for (int aWave = 0; aWave < mBoardData.mNumWaves; aWave++) {
        ZombiePickerInitForWave(&aZombiePicker);
        mBoardData.mZombiesInWave[aWave][0] = ZombieType::ZOMBIE_INVALID;

        const bool aIsFlagWave = IsFlagWave(aWave);
        const bool aIsFinalWave = aWave == mBoardData.mNumWaves - 1;

        if (mApp->IsBungeeBlitzLevel() && aIsFlagWave) {
            // 蹦极闪电战关卡的每大波固定刷出 5 只蹦极僵尸
            for (int _i = 0; _i < 5; _i++)
                PutZombieInWave(ZombieType::ZOMBIE_BUNGEE, aWave, &aZombiePicker);

            if (!aIsFinalWave) continue;
        }

        // ------------------------------------------------------------------------------------------------
        // △ 计算该波的僵尸总点数
        // ------------------------------------------------------------------------------------------------
        int &aZombiePoints = aZombiePicker.mZombiePoints;
        // 根据关卡计算本波的基础僵尸点数
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
            aZombiePoints = (mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + aWave + 10) * 2 / 5 + 1;
        } else if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0) {
            aZombiePoints = (mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + aWave) * 2 / 5 + 1;
        } else if (mApp->IsAdventureMode() && mApp->HasFinishedAdventure() && mBoardData.mLevel != 5) {
            aZombiePoints = aWave * 2 / 5 + 1;
        } else {
            aZombiePoints = aWave / 3 + 1;
        }

        // 旗帜波的特殊调整
        if (aIsFlagWave) {
            const int aPlainZombiesNum = std::min(aZombiePoints, 8);
            aZombiePoints *= 2.5f;

            if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS &&
                mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2) {
                for (int _i = 0; _i < aPlainZombiesNum; _i++) {
                    PutZombieInWave(ZombieType::ZOMBIE_NORMAL, aWave, &aZombiePicker);
                }
                PutZombieInWave(ZombieType::ZOMBIE_FLAG, aWave, &aZombiePicker);
            }
        }

        // 部分关卡的多倍出怪
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN) {
            aZombiePoints *= 6;
        } else if (mApp->IsLittleTroubleLevel() || mApp->IsWallnutBowlingLevel()) {
            aZombiePoints *= 4;
        } else if (mApp->IsMiniBossLevel()) {
            aZombiePoints *= 3;
        } else if (mApp->IsStormyNightLevel() && mApp->IsAdventureMode()) {
            aZombiePoints *= 3;
        } else if (mApp->IsShovelLevel() || mApp->IsBungeeBlitzLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL) {
            aZombiePoints *= 2;
        }

        // ------------------------------------------------------------------------------------------------
        // △ 向出怪列表中加入固定刷出的僵尸
        // ------------------------------------------------------------------------------------------------
        // 部分新出现的僵尸会在特定波固定刷出
        if (aIntroZombieType != ZombieType::ZOMBIE_INVALID && aIntroZombieType != ZombieType::ZOMBIE_DUCKY_TUBE) {
            bool aSpawnIntro = false;
            if ((aIntroZombieType == ZombieType::ZOMBIE_DIGGER || aIntroZombieType == ZombieType::ZOMBIE_BALLOON)) {
                if (aWave + 1 == 7 || aIsFinalWave) {
                    aSpawnIntro = true;
                }
            } else if (aIntroZombieType == ZombieType::ZOMBIE_YETI) {
                if (aWave == mBoardData.mNumWaves / 2 && !mApp->mSawYeti) {
                    aSpawnIntro = true;
                }
            } else if (aWave == mBoardData.mNumWaves / 2 || aIsFinalWave) {
                aSpawnIntro = true;
            }

            if (aSpawnIntro) {
                PutZombieInWave(aIntroZombieType, aWave, &aZombiePicker);
            }
        }

        // 5-10 关卡的最后一波加入一只伽刚特尔
        if (mBoardData.mLevel == 50 && aIsFinalWave) {
            PutZombieInWave(ZombieType::ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
        }
        // 冒险模式关卡的最后一波会出现本关卡可能出现的所有僵尸
        if (mApp->IsAdventureMode() && aIsFinalWave) {
            PutInMissingZombies(aWave, &aZombiePicker);
        }
        // 柱子关卡的特殊出怪
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN) {
            // 每大波的第 5 小波，固定出现 10 只扶梯僵尸
            if (aWave % 10 == 5) {
                for (int _i = 0; _i < 10; _i++) {
                    PutZombieInWave(ZombieType::ZOMBIE_LADDER, aWave, &aZombiePicker);
                }
            }

            // 每大波的第 8 小波，固定出现 10 只玩偶匣僵尸
            if (aWave % 10 == 8) {
                for (int _i = 0; _i < 10; _i++) {
                    PutZombieInWave(ZombieType::ZOMBIE_JACK_IN_THE_BOX, aWave, &aZombiePicker);
                }
            }

            // 第 19/29 小波，固定出现 3/5 只伽刚特尔
            if (aWave == 19) {
                for (int _i = 0; _i < 3; _i++) {
                    PutZombieInWave(ZombieType::ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
                }
            }
            if (aWave == 29) {
                for (int _i = 0; _i < 5; _i++) {
                    PutZombieInWave(ZombieType::ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
                }
            }
        }

        // ------------------------------------------------------------------------------------------------
        // △ 剩余的僵尸点数用于向列表中补充随机僵尸
        // ------------------------------------------------------------------------------------------------
        while (aZombiePoints > 0 && aZombiePicker.mZombieCount < MAX_ZOMBIES_IN_WAVE) {
            const ZombieType aZombieType = PickZombieType(aZombiePoints, aWave, &aZombiePicker);
            PutZombieInWave(aZombieType, aWave, &aZombiePicker);
        }
    }
}

// 0x40A110
int Board::GetLevelRandSeed() const {
    int aRndSeed = mApp->mPlayerInfo->mId + mBoardData.mBoardRandSeed;
    if (mApp->IsAdventureMode()) {
        aRndSeed += mApp->mPlayerInfo->mFinishedAdventure * 101 + mBoardData.mLevel;
    } else {
        aRndSeed += mChallenge->mSurvivalStage * 101 + mApp->mGameMode;
    }
    return aRndSeed;
}

// 0x40A160
//  GOTY @Patoke: 0x40C9F0
void Board::LoadBackgroundImages() const {
    switch (mBoardData.mBackground) {
    case BackgroundType::BACKGROUND_1_DAY:
        TodLoadResources("DelayLoad_Background1");
        if ((mApp->IsAdventureMode() && mBoardData.mLevel <= 4) ||
            mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED) {
            TodLoadResources("DelayLoad_BackgroundUnsodded");
        }
        break;

    case BackgroundType::BACKGROUND_2_NIGHT: TodLoadResources("DelayLoad_Background2"); break;

    case BackgroundType::BACKGROUND_3_POOL: TodLoadResources("DelayLoad_Background3"); break;

    case BackgroundType::BACKGROUND_4_FOG: TodLoadResources("DelayLoad_Background4"); break;

    case BackgroundType::BACKGROUND_5_ROOF: TodLoadResources("DelayLoad_Background5"); break;

    case BackgroundType::BACKGROUND_6_BOSS: TodLoadResources("DelayLoad_Background6"); break;

    case BackgroundType::BACKGROUND_GREENHOUSE:
        TodLoadResources("DelayLoad_GreenHouseGarden");
        TodLoadResources("DelayLoad_GreenHouseOverlay");
        break;

    case BackgroundType::BACKGROUND_TREEOFWISDOM:
        ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_TREEOFWISDOM, true);
        break;

    case BackgroundType::BACKGROUND_ZOMBIQUARIUM:
        TodLoadResources("DelayLoad_Zombiquarium");
        TodLoadResources("DelayLoad_GreenHouseOverlay");
        break;

    case BackgroundType::BACKGROUND_MUSHROOM_GARDEN: TodLoadResources("DelayLoad_MushroomGarden"); break;

    default: TOD_ASSERT(); break;
    }
}

// 0x40A550
void Board::PickBackground() {
    switch (mApp->mGameMode) {
    case GameMode::GAMEMODE_ADVENTURE:
        if (mBoardData.mLevel <= 1 * LEVELS_PER_AREA) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_1_DAY;
        } else if (mBoardData.mLevel <= 2 * LEVELS_PER_AREA) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_2_NIGHT;
        } else if (mBoardData.mLevel <= 3 * LEVELS_PER_AREA) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_3_POOL;
        } else if (mApp->IsScaryPotterLevel()) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_2_NIGHT;
        } else if (mBoardData.mLevel <= 4 * LEVELS_PER_AREA) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_4_FOG;
        } else if (mBoardData.mLevel < FINAL_LEVEL) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_5_ROOF;
        } else if (mBoardData.mLevel == FINAL_LEVEL) {
            mBoardData.mBackground = BackgroundType::BACKGROUND_6_BOSS;
        } else {
            mBoardData.mBackground = BackgroundType::BACKGROUND_1_DAY;
        }
        break;

    case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_1:
    case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_1:
    case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_1:
    case GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS:
    case GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING:
    case GameMode::GAMEMODE_CHALLENGE_SLOT_MACHINE:
    case GameMode::GAMEMODE_CHALLENGE_SEEING_STARS:
    case GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2:
    case GameMode::GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT:
    case GameMode::GAMEMODE_CHALLENGE_SUNNY_DAY:
    case GameMode::GAMEMODE_CHALLENGE_RESODDED:
    case GameMode::GAMEMODE_CHALLENGE_BIG_TIME:
    case GameMode::GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER:
    case GameMode::GAMEMODE_CHALLENGE_ICE:
    case GameMode::GAMEMODE_CHALLENGE_SHOVEL:
    case GameMode::GAMEMODE_CHALLENGE_SQUIRREL:                mBoardData.mBackground = BackgroundType::BACKGROUND_1_DAY; break;

    case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_2:
    case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_2:
    case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_2:
    case GameMode::GAMEMODE_CHALLENGE_BEGHOULED:
    case GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST:
    case GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT:
    case GameMode::GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE:
    case GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER:
    case GameMode::GAMEMODE_SCARY_POTTER_1:
    case GameMode::GAMEMODE_SCARY_POTTER_2:
    case GameMode::GAMEMODE_SCARY_POTTER_3:
    case GameMode::GAMEMODE_SCARY_POTTER_4:
    case GameMode::GAMEMODE_SCARY_POTTER_5:
    case GameMode::GAMEMODE_SCARY_POTTER_6:
    case GameMode::GAMEMODE_SCARY_POTTER_7:
    case GameMode::GAMEMODE_SCARY_POTTER_8:
    case GameMode::GAMEMODE_SCARY_POTTER_9:
    case GameMode::GAMEMODE_SCARY_POTTER_ENDLESS:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9:
    case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS:   mBoardData.mBackground = BackgroundType::BACKGROUND_2_NIGHT; break;

    case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_3:
    case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_3:
    case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_3:
    case GameMode::GAMEMODE_CHALLENGE_LITTLE_TROUBLE:
    case GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA:
    case GameMode::GAMEMODE_CHALLENGE_SPEED:
    case GameMode::GAMEMODE_CHALLENGE_LAST_STAND:
    case GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2:
    case GameMode::GAMEMODE_UPSELL:
    case GameMode::GAMEMODE_INTRO:                     mBoardData.mBackground = BackgroundType::BACKGROUND_3_POOL; break;

    case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_4:
    case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_4:
    case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_4:
    case GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS:
    case GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL:
    case GameMode::GAMEMODE_CHALLENGE_AIR_RAID:
    case GameMode::GAMEMODE_CHALLENGE_STORMY_NIGHT:   mBoardData.mBackground = BackgroundType::BACKGROUND_4_FOG; break;

    case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_5:
    case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_5:
    case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_5:
    case GameMode::GAMEMODE_CHALLENGE_COLUMN:
    case GameMode::GAMEMODE_CHALLENGE_POGO_PARTY:
    case GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY:
    case GameMode::GAMEMODE_CHALLENGE_BUNGEE_BLITZ:   mBoardData.mBackground = BackgroundType::BACKGROUND_5_ROOF; break;

    case GameMode::GAMEMODE_CHALLENGE_FINAL_BOSS: mBoardData.mBackground = BackgroundType::BACKGROUND_6_BOSS; break;

    case GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM:
        mBoardData.mBackground = BackgroundType::BACKGROUND_ZOMBIQUARIUM;
        break;

    case GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN: mBoardData.mBackground = BackgroundType::BACKGROUND_GREENHOUSE; break;

    case GameMode::GAMEMODE_TREE_OF_WISDOM: mBoardData.mBackground = BackgroundType::BACKGROUND_TREEOFWISDOM; break;

    default: TOD_ASSERT(); break;
    }
    LoadBackgroundImages();

    if (mBoardData.mBackground == BackgroundType::BACKGROUND_1_DAY ||
        mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
        mBoardData.mBackground == BackgroundType::BACKGROUND_TREEOFWISDOM) {
        mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[5] = PlantRowType::PLANTROW_DIRT;

        if (mApp->IsAdventureMode() && mApp->IsFirstTimeAdventureMode()) {
            if (mBoardData.mLevel == 1) {
                mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_DIRT;
                mBoardData.mPlantRow[1] = PlantRowType::PLANTROW_DIRT;
                mBoardData.mPlantRow[3] = PlantRowType::PLANTROW_DIRT;
                mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_DIRT;
            } else if (mBoardData.mLevel == 2 || mBoardData.mLevel == 3) {
                mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_DIRT;
                mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_DIRT;
            }
        } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED) {
            mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_DIRT;
            mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_DIRT;
        }
    } else if (mBoardData.mBackground == BackgroundType::BACKGROUND_2_NIGHT) {
        mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[5] = PlantRowType::PLANTROW_DIRT;
    } else if (mBoardData.mBackground == BackgroundType::BACKGROUND_3_POOL ||
               mBoardData.mBackground ==
               BackgroundType::BACKGROUND_ZOMBIQUARIUM || mBoardData.mBackground
               == BackgroundType::BACKGROUND_4_FOG) {
        mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[2] = PlantRowType::PLANTROW_POOL;
        mBoardData.mPlantRow[3] = PlantRowType::PLANTROW_POOL;
        mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[5] = PlantRowType::PLANTROW_NORMAL;
    } else if (mBoardData.mBackground == BackgroundType::BACKGROUND_5_ROOF || mBoardData.mBackground == BackgroundType::BACKGROUND_6_BOSS) {
        mBoardData.mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
        mBoardData.mPlantRow[5] = PlantRowType::PLANTROW_DIRT;
    } else {
        TOD_ASSERT();
    }

    for (int x = 0; x < MAX_GRID_SIZE_X; x++) {
        for (int y = 0; y < MAX_GRID_SIZE_Y; y++) {
            if (mBoardData.mPlantRow[y] == PlantRowType::PLANTROW_DIRT) {
                mBoardData.mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_DIRT;
            } else if (mBoardData.mPlantRow[y] == PlantRowType::PLANTROW_POOL && x >= 0 && x <= 8) {
                mBoardData.mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_POOL;
            } else if (mBoardData.mPlantRow[y] == PlantRowType::PLANTROW_HIGH_GROUND && x >= 4 && x <= 8) {
                mBoardData.mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_HIGH_GROUND;
            }
        }
    }

    MTRand aLevelRNG(GetLevelRandSeed());
    if (StageHasGraveStones()) {
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER) {
            AddGraveStones(6, RandRangeInt(1, 2), aLevelRNG);
            AddGraveStones(7, RandRangeInt(1, 3), aLevelRNG);
            AddGraveStones(8, RandRangeInt(2, 3), aLevelRNG);
        } else if (mApp->IsWhackAZombieLevel()) {
            mChallenge->WhackAZombiePlaceGraves(9);
        } else if (mBoardData.mBackground == BackgroundType::BACKGROUND_2_NIGHT) {
            if (mApp->IsSurvivalNormal(mApp->mGameMode)) {
                AddGraveStones(5, 1, aLevelRNG);
                AddGraveStones(6, 1, aLevelRNG);
                AddGraveStones(7, 1, aLevelRNG);
                AddGraveStones(8, 2, aLevelRNG);
            } else if (!mApp->IsAdventureMode()) {
                AddGraveStones(4, 1, aLevelRNG);
                AddGraveStones(5, 1, aLevelRNG);
                AddGraveStones(6, 2, aLevelRNG);
                AddGraveStones(7, 2, aLevelRNG);
                AddGraveStones(8, 3, aLevelRNG);
            } else if (mBoardData.mLevel == 11 || mBoardData.mLevel == 12 || mBoardData.mLevel == 13) {
                AddGraveStones(6, 1, aLevelRNG);
                AddGraveStones(7, 1, aLevelRNG);
                AddGraveStones(8, 2, aLevelRNG);
            } else if (mBoardData.mLevel == 14 || mBoardData.mLevel == 16) {
                AddGraveStones(5, 1, aLevelRNG);
                AddGraveStones(6, 1, aLevelRNG);
                AddGraveStones(7, 2, aLevelRNG);
                AddGraveStones(8, 3, aLevelRNG);
            } else if (mBoardData.mLevel == 17 || mBoardData.mLevel == 18 || mBoardData.mLevel == 19) {
                AddGraveStones(4, 1, aLevelRNG);
                AddGraveStones(5, 2, aLevelRNG);
                AddGraveStones(6, 2, aLevelRNG);
                AddGraveStones(7, 3, aLevelRNG);
                AddGraveStones(8, 3, aLevelRNG);
            } else if (mBoardData.mLevel >= 20) {
                AddGraveStones(3, 1, aLevelRNG);
                AddGraveStones(4, 2, aLevelRNG);
                AddGraveStones(5, 2, aLevelRNG);
                AddGraveStones(6, 2, aLevelRNG);
                AddGraveStones(7, 3, aLevelRNG);
                AddGraveStones(8, 3, aLevelRNG);
            } else {
                TOD_ASSERT();
            }
        }
    }
    PickSpecialGraveStone();
}

// 0x40AB10
void Board::InitZombieWavesForLevel(const int theForLevel) {
    if (mApp->IsWhackAZombieLevel() || (mApp->IsWallnutBowlingLevel() && !mApp->IsFirstTimeAdventureMode())) {
        mChallenge->InitZombieWaves();
        return;
    }

    for (int aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType++) {
        mBoardData.mZombieAllowed[aZombieType] =
            CanZombieSpawnOnLevel(static_cast<ZombieType>(aZombieType), theForLevel);
    }
}

bool Board::IsZombieWaveDistributionOk() const {
    if (!mApp->IsAdventureMode()) return true;

    int aZombieTypeCount[static_cast<int>(ZombieType::NUM_ZOMBIE_TYPES)] = {0};
    for (int aWave = 0; aWave < mBoardData.mNumWaves; aWave++) {
        for (int aIndex = 0; aIndex < MAX_ZOMBIES_IN_WAVE; aIndex++) {
            const ZombieType aZombieType = mBoardData.mZombiesInWave[aWave][aIndex];
            if (aZombieType == ZombieType::ZOMBIE_INVALID) {
                break;
            }

            TOD_ASSERT(aZombieType >= 0 && aZombieType < ZombieType::NUM_ZOMBIE_TYPES);
            aZombieTypeCount[static_cast<int>(aZombieType)]++;
        }
    }

    for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES;
         aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1)) {
        if (aZombieType != ZombieType::ZOMBIE_YETI && CanZombieSpawnOnLevel(aZombieType, mBoardData.mLevel) &&
            aZombieTypeCount[static_cast<int>(aZombieType)] == 0) {
            TodTraceAndLog(
                "Didn't spawn required zombie {}, level {}",
                SexyStringToStringFast(GetZombieDefinition(aZombieType).mZombieName), mBoardData.mLevel
            );
            return false;
        }
    }
    return true;
}

// 0x40ABB0
void Board::InitZombieWaves() {
    memset(mBoardData.mZombieAllowed, false, sizeof(mBoardData.mZombieAllowed));
    if (mApp->IsAdventureMode()) {
        InitZombieWavesForLevel(mBoardData.mLevel);
    } else {
        mChallenge->InitZombieWaves();
    }
    PickZombieWaves();
    TOD_ASSERT(IsZombieWaveDistributionOk());

    mBoardData.mCurrentWave = 0;
    mBoardData.mTotalSpawnedWaves = 0;
    mApp->mSawYeti = false;
    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 2) {
        mBoardData.mZombieCountDown = ZOMBIE_COUNTDOWN * 2;
    } else if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0) {
        mBoardData.mZombieCountDown = ZOMBIE_COUNTDOWN_RANGE;
    } else {
        mBoardData.mZombieCountDown = ZOMBIE_COUNTDOWN_FIRST_WAVE;
    }

    mBoardData.mZombieHealthWaveStart = 0;
    mBoardData.mLastBungeeWave = 0;
    mBoardData.mProgressMeterWidth = 0;
    mBoardData.mHugeWaveCountDown = 0;
    mBoardData.mLevelAwardSpawned = false;
    mBoardData.mZombieCountDownStart = mBoardData.mZombieCountDown;
    mBoardData.mZombieHealthToNextWave = -1;
}

// 0x40ACB0
void Board::FreezeEffectsForCutscene(const bool theFreeze) {
    TodParticleSystem *aParticle = nullptr;
    while (IterateParticles(aParticle)) {
        if (aParticle->mEffectType == ParticleEffect::PARTICLE_GRAVE_BUSTER) {
            if (aParticle->mEffectType == ParticleEffect::PARTICLE_POOL_SPARKLY && mBoardData.mIceTrapCounter == 0) {
                aParticle->mDontUpdate = theFreeze;
            }
        }
    }

    Reanimation *aReanim = nullptr;
    while (IterateReanimations(aReanim)) {
        if (aReanim->mReanimationType == ReanimationType::REANIM_SLEEPING) {
            aReanim->mAnimRate = theFreeze ? 0.0f : RandRangeFloat(6, 8);
        }
    }
}

// 0x40AD60
void Board::InitSurvivalStage() {
    RefreshSeedPacketFromCursor();
    mApp->mSoundSystem->GamePause(true);
    FreezeEffectsForCutscene(true);
    mBoardData.mLevelComplete = false;
    InitZombieWaves();
    mApp->mGameScene = GameScenes::SCENE_LEVEL_INTRO;
    mApp->ShowSeedChooserScreen();
    mCutScene->StartLevelIntro();
    mSeedBank->UpdateWidth();

    for (int i = 0; i < SEEDBANK_MAX; i++) {
        SeedPacket *aPacket = &mSeedBank->mSeedPackets[i];
        aPacket->mX = GetSeedPacketPositionX(i);
        aPacket->mPacketType = SeedType::SEED_NONE;
    }

    if (StageHasFog()) {
        mBoardData.mFogBlownCountDown = FOG_BLOW_RETURN_TIME;
    }
    for (int j = 0; j < MAX_GRID_SIZE_Y; j++) {
        mBoardData.mWaveRowGotLawnMowered[j] = -100;
    }
}

// 0x40AE70
Rect Board::GetShovelButtonRect() {
    Rect aRect(GetSeedBankExtraWidth() + 456, 0, Sexy::IMAGE_SHOVELBANK->GetWidth(), Sexy::IMAGE_SEEDBANK->GetHeight());
    if (mApp->IsSlotMachineLevel() || mApp->IsSquirrelLevel()) {
        aRect.mX = 600;
    }
    return aRect;
}

// 0x40AF00
void Board::GetZenButtonRect(const GameObjectType theObjectType, Rect &theRect) {
    // 此函数与内测版的差异在于，内测版在此函数中通过下列语句先取得了铲子按钮矩形：
    // Rect aRect = GetShovelButtonRect();
    // 而原版需要在函数调用前先自行取得铲子按钮矩形，并将该矩形作为参数传递给此函数，
    // 原版中此函数有将 theRect 的引用作为返回值，但并无直接使用返回值的情况。
    // 此处为了防止误用返回值而出现问题，故删除其返回值，如需调用可按照如下方式：
    // Rect aButtonRect = GetShovelButtonRect();
    // GetZenButtonRect(xxx, aButtonRect);

    // @ Minerscale Zen Garden button locations
    if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        theRect.mX = 30;
        /*
        if (theObjectType == GameObjectType::OBJECT_TYPE_NEXT_GARDEN)
        {
            theRect.mX = 30;
        }
        //return theRect;
        */
    } else {
        theRect.mX = 0;
    }

    bool usable = true;
    for (int anObject = GameObjectType::OBJECT_TYPE_WATERING_CAN; anObject <= GameObjectType::OBJECT_TYPE_NEXT_GARDEN;
         anObject++) {
        // 只要有一个按钮不可用，则所有可用按钮排列的起始横坐标为 30
        if (!CanUseGameObject(static_cast<GameObjectType>(anObject))) {
            usable = false;
        }
    }
    if (usable) {
        theRect.mX = 0;
    }

    for (int anObject = GameObjectType::OBJECT_TYPE_WATERING_CAN; anObject < theObjectType; anObject++) {
        // 每存在一个序号小于目标的可用按钮，则目标按钮的横坐标增加 70
        if (CanUseGameObject(static_cast<GameObjectType>(anObject))) {
            theRect.mX += 70; // Sexy::IMAGE_SHOVELBANK->GetWidth();
        }
    }
    // return theRect;
}

// 0x40AF90
//  GOTY @Patoke: 0x40D840
void Board::InitLevel() {
    mBoardData.mMainCounter = 0;
    mBoardData.mEnableGraveStones = false;
    mBoardData.mSodPosition = 0;
    mBoardData.mPrevBoardResult = mApp->mBoardResult;

    const GameMode aGameMode = mApp->mGameMode;
    if (aGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM && aGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        mApp->mMusic->StopAllMusic();
    }
    // 赋值当前关卡
    mBoardData.mLevel = mApp->IsAdventureMode() ? mApp->mPlayerInfo->mLevel : 0;
    // 设定关卡背景
    PickBackground();
    // 设定关卡出怪
    InitZombieWaves();
    // 设定关卡初始阳光数量
    if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST || mApp->IsScaryPotterLevel() ||
        mApp->IsWhackAZombieLevel()) {
        mBoardData.mSunMoney = 0;
    } else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
        mBoardData.mSunMoney = 5000;
    } else if (mApp->IsIZombieLevel()) {
        mBoardData.mSunMoney = 150;
    } else if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 1) {
        mBoardData.mSunMoney = 150;
    } else {
        mBoardData.mSunMoney = 50;
    }

    // 初始化行选择数组
    memset(mBoardData.mRowPickingArray, 0, sizeof(mBoardData.mRowPickingArray));
    // 初始化每行的基础数据
    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        mBoardData.mWaveRowGotLawnMowered[aRow] = -100;
        mBoardData.mIceMinX[aRow] = BOARD_ICE_START;
        mBoardData.mIceTimer[aRow] = 0;
        mBoardData.mIceParticleID[aRow] = ParticleSystemID::PARTICLESYSTEMID_NULL;
        mBoardData.mRowPickingArray[aRow].mItem = aRow;
    }
    // 初始化阳光掉落
    mBoardData.mNumSunsFallen = 0;
    if (!StageIsNight()) {
        mBoardData.mSunCountDown = RandRangeInt(425, 700);
    }
    // 初始化字幕播放记录
    memset(mBoardData.mHelpDisplayed, 0, sizeof(mBoardData.mHelpDisplayed));
    // 初始化卡槽及卡牌
    mSeedBank->mNumPackets = GetNumSeedsInBank();
    mSeedBank->UpdateWidth();
    for (int i = 0; i < SEEDBANK_MAX; i++) {
        SeedPacket *aPacket = &mSeedBank->mSeedPackets[i];
        aPacket->mIndex = i;
        aPacket->mX = GetSeedPacketPositionX(i);
        aPacket->mY = 8;
        aPacket->mPacketType = SeedType::SEED_NONE;
    }
    // 设定固定卡牌
    if (mApp->IsSlotMachineLevel()) {
        TOD_ASSERT(mSeedBank->mNumPackets == 3);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_SUNFLOWER);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_PEASHOOTER);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_SNOWPEA);
    } else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_ICE) {
        TOD_ASSERT(mSeedBank->mNumPackets == 6);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_PEASHOOTER);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_CHERRYBOMB);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_WALLNUT);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_REPEATER);
        mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_SNOWPEA);
        mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_CHOMPER);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1) {
        TOD_ASSERT(mSeedBank->mNumPackets == 3);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_FOOTBALL);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2) {
        TOD_ASSERT(mSeedBank->mNumPackets == 3);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_SCREEN_DOOR);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3) {
        TOD_ASSERT(mSeedBank->mNumPackets == 3);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4) {
        TOD_ASSERT(mSeedBank->mNumPackets == 3);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5) {
        TOD_ASSERT(mSeedBank->mNumPackets == 4);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_BALLOON);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6) {
        TOD_ASSERT(mSeedBank->mNumPackets == 4);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_GARGANTUAR);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7) {
        TOD_ASSERT(mSeedBank->mNumPackets == 4);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_DANCER);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8) {
        TOD_ASSERT(mSeedBank->mNumPackets == 6);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_IMP);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_TRAFFIC_CONE);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
        mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
        mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9) {
        TOD_ASSERT(mSeedBank->mNumPackets == 8);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_IMP);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_TRAFFIC_CONE);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
        mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
        mSeedBank->mSeedPackets[6].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
        mSeedBank->mSeedPackets[7].SetPacketType(SeedType::SEED_ZOMBIE_FOOTBALL);
    } else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS) {
        TOD_ASSERT(mSeedBank->mNumPackets == 9);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_IMP);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_TRAFFIC_CONE);
        mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
        mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
        mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
        mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
        mSeedBank->mSeedPackets[6].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
        mSeedBank->mSeedPackets[7].SetPacketType(SeedType::SEED_ZOMBIE_FOOTBALL);
        mSeedBank->mSeedPackets[8].SetPacketType(SeedType::SEED_ZOMBIE_DANCER);
    } else if (mApp->IsScaryPotterLevel()) {
        TOD_ASSERT(mSeedBank->mNumPackets == 1);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_CHERRYBOMB);
    } else if (mApp->IsWhackAZombieLevel()) {
        TOD_ASSERT(mSeedBank->mNumPackets == 3);
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_POTATOMINE);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_GRAVEBUSTER);
        mSeedBank->mSeedPackets[2].SetPacketType(
            mApp->IsAdventureMode() ? SeedType::SEED_CHERRYBOMB : SeedType::SEED_ICESHROOM
        );
    } else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM) {
        mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIQUARIUM_SNORKLE);
        mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIQUARIUM_TROPHY);
    } else if (!ChooseSeedsOnCurrentLevel() && !HasConveyorBeltSeedBank()) {
        mSeedBank->mNumPackets = GetNumSeedsInBank();
        // 卡槽错误的关卡，依次填充所有卡牌
        for (int i = 0; i < mSeedBank->mNumPackets; i++) {
            mSeedBank->mSeedPackets[i].SetPacketType(static_cast<SeedType>(i));
        }
    }
    // 将所有子控件标记为已变动
    MarkAllDirty();

    mBoardData.mPaused = false;
    mBoardData.mOutOfMoneyCounter = 0;
    if (StageHasFog()) {
        mBoardData.mFogBlownCountDown = 200;
        mBoardData.mFogOffset = 1065 - LeftFogColumn() * 80;
    }
    // 关卡玩法相关的初始化
    mChallenge->InitLevel();
}

Reanimation *Board::CreateRakeReanim(const float theRakeX, const float theRakeY, const int theRenderOrder) const {
    Reanimation *aReanim = mApp->AddReanimation(theRakeX + 20, theRakeY, theRenderOrder, REANIM_RAKE);
    aReanim->mAnimRate = 0;
    aReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
    aReanim->mIsAttachment = true;
    return aReanim;
}

// 0x40B9C0
void Board::PlaceRake() {
    if (!mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_RAKE)]) return;

    int aGridX = 7;
    if (mApp->IsScaryPotterLevel()) {
        GridItem *aGridItem = nullptr;
        while (IterateGridItems(aGridItem)) {
            if (aGridItem->mGridItemType == GridItemType::GRIDITEM_SCARY_POT && aGridItem->mGridX <= aGridX &&
                aGridItem->mGridX > 0) {
                aGridX = aGridItem->mGridX - 1;
            }
        }
    } else {
        if (!StageHasZombieWalkInFromRight() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
            mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
            mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
            return;
    }

    int aPickCount = 0;
    TodWeightedArray<int> aPickArray[MAX_GRID_SIZE_Y];
    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        if (aRow != 5 && mBoardData.mPlantRow[aRow] == PlantRowType::PLANTROW_NORMAL) {
            aPickArray[aPickCount].mWeight = 1;
            aPickArray[aPickCount].mItem = aRow;
            aPickCount++;
        }
    }
    if (aPickCount == 0) return;

    const int aGridY = TodPickFromWeightedArray(aPickArray, aPickCount);
    mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_RAKE)]--;
    GridItem *aRake = mGridItems.DataArrayAlloc();
    aRake->mGridItemType = GridItemType::GRIDITEM_RAKE;
    aRake->mGridX = aGridX;
    aRake->mGridY = aGridY;
    aRake->mPosX = GridToPixelX(aGridX, aGridY);
    aRake->mPosY = GridToPixelY(aGridX, aGridY);
    aRake->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, aGridY, 9);
    aRake->mGridItemReanimID = mApp->ReanimationGetID(CreateRakeReanim(aRake->mPosX, aRake->mPosY, 0));
    // Lmao gotta pass in the right coords
    aRake->mGridItemState = GridItemState::GRIDITEM_STATE_RAKE_ATTRACTING;
}

// 0x40BC70
void Board::InitLawnMowers() {
    const GameMode aGameMode = mApp->mGameMode;
    // 这里优化一下原版的代码，事先列举一些不创建小推车的关卡
    if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
        aGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || aGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
        aGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND ||
        aGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || mApp->IsSquirrelLevel() || mApp->IsIZombieLevel() ||
        (StageHasRoof() && !mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_ROOF_CLEANER]))
        return;

    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        if ((aGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED && aRow <= 4) ||
            (mApp->IsAdventureMode() && mBoardData.mLevel == 35) ||
            // 这里原版没有对于行的判断，故冒险模式 4-5 关卡有 6 行小推车
            (!mApp->IsScaryPotterLevel() && mBoardData.mPlantRow[aRow] != PlantRowType::PLANTROW_DIRT))
        // 除冒险模式 4-5 关卡外的破罐者模式关卡无小推车
        {
            LawnMower *aLawnMower = mLawnMowers.DataArrayAlloc();
            aLawnMower->LawnMowerInitialize(aRow);
            aLawnMower->mVisible = false;
        }
    }
}

// 0x40BD30
bool Board::ChooseSeedsOnCurrentLevel() {
    if (mApp->IsChallengeWithoutSeedBank() || HasConveyorBeltSeedBank()) return false;

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
        return false;

    if (mApp->IsIZombieLevel() || mApp->IsSlotMachineLevel()) return false;

    return (!mApp->IsFirstTimeAdventureMode() || mBoardData.mLevel > 7);
}

// 0x40BE00
//  GOTY @Patoke: 0x40E6A0
void Board::StartLevel() {
    mBoardData.mCoinBankFadeCount = 0;
    mApp->mLastLevelStats->Reset();
    mChallenge->StartLevel();

    // @Patoke: implemented, i think it's intentional to cause an underflow here?
    const unsigned int aSurvivalStage = mApp->mGameMode - GAMEMODE_SURVIVAL_ENDLESS_STAGE_1;
    if (aSurvivalStage <= 4) {
        if (GetSurvivalFlagsCompleted() >= 20) {
            // if ( !*(mApp->mPlayerInfo + 53) ) todo @Patoke: add this?
            ReportAchievement::GiveAchievement(mApp, Immortal, true);
        }
    }

    if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0) {
        mApp->EraseFile(GetSavedGameName(mApp->mGameMode, mApp->mPlayerInfo->mId));
        FreezeEffectsForCutscene(false);
        mApp->mSoundSystem->GamePause(false);
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || mApp->mGameMode == GameMode::GAMEMODE_UPSELL ||
        mApp->mGameMode == GameMode::GAMEMODE_INTRO || mApp->IsFinalBossLevel())
        return;

    mApp->mMusic->StartGameMusic();
}

// 0x40BF10
LawnMower *Board::GetBottomLawnMower() {
    LawnMower *aLawnMower = nullptr;
    LawnMower *aBottomMower = nullptr;
    while (IterateLawnMowers(aLawnMower)) {
        if (aLawnMower->mMowerState == LawnMowerState::MOWER_TRIGGERED ||
            aLawnMower->mMowerState == LawnMowerState::MOWER_SQUISHED)
            continue;

        if (aBottomMower == nullptr || aBottomMower->mRow < aLawnMower->mRow) {
            aBottomMower = aLawnMower;
        }
    }
    return aBottomMower;
}

// 0x40BF60
//  GOTY @Patoke: 0x40E860
void Board::UpdateLevelEndSequence() {
    if (mBoardData.mNextSurvivalStageCounter > 0) {
        if (!IsScaryPotterDaveTalking()) {
            mBoardData.mNextSurvivalStageCounter--;
            if (mApp->IsAdventureMode() && mApp->IsScaryPotterLevel() && mBoardData.mNextSurvivalStageCounter == 300) {
                mApp->CrazyDaveEnter();
                mApp->CrazyDaveTalkIndex(mChallenge->mSurvivalStage == 0 ? 2700 : 2800);
                mChallenge->PuzzleNextStageClear();
                mBoardData.mNextSurvivalStageCounter = 100;
            }
        }

        if (mBoardData.mNextSurvivalStageCounter == 1 && mApp->IsSurvivalMode()) {
            TryToSaveGame();
        }

        if (!mBoardData.mNextSurvivalStageCounter) {
            if (mApp->IsScaryPotterLevel()) {
                if (mApp->IsAdventureMode()) return;

                if (!IsFinalScaryPotterStage()) {
                    mChallenge->PuzzleNextStageClear();
                    mChallenge->ScaryPotterPopulate();
                }
            } else if (LawnApp::IsEndlessIZombie(mApp->mGameMode)) {
                mChallenge->PuzzleNextStageClear();
                mChallenge->IZombieInitLevel();
            } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
                ClearAdvice(AdviceType::ADVICE_NONE);
            } else {
                mBoardData.mLevelComplete = true;
                RemoveZombiesForRepick();
            }
            return;
        }
    }

    if (mBoardData.mBoardFadeOutCounter < 0) return;

    mBoardData.mBoardFadeOutCounter--;
    if (mBoardData.mBoardFadeOutCounter == 0) {
        mBoardData.mLevelComplete = true;
        return;
    }
    if (mBoardData.mBoardFadeOutCounter == 300) {
        if (!IsSurvivalStageWithRepick() &&
            !(mBoardData.mLevel == 9 || mBoardData.mLevel == 19 || mBoardData.mLevel == 29 || mBoardData.mLevel == 39 ||
              mBoardData.mLevel == 49)) {
            mApp->PlaySample(Sexy::SOUND_LIGHTFILL);
        }
    }

    if (mBoardData.mScoreNextMowerCounter > 0) {
        mBoardData.mScoreNextMowerCounter--;
        if (mBoardData.mScoreNextMowerCounter) {
            return;
        }
    }

    if (CanDropLoot() && !IsSurvivalStageWithRepick()) {
        mBoardData.mScoreNextMowerCounter = 40;
        LawnMower *aLawnMower = GetBottomLawnMower();
        if (aLawnMower) {
            AddCoin(
                aLawnMower->mPosX + 40, aLawnMower->mPosY + 40, CoinType::COIN_GOLD,
                CoinMotion::COIN_MOTION_LAWNMOWER_COIN
            );
            SoundInstance *aSoundInstance = mApp->mSoundManager->GetSoundInstance(Sexy::SOUND_POINTS);
            if (aSoundInstance) {
                aSoundInstance->Play(false, true);
                const float aPitch = ClampFloat(6 - CountUntriggerLawnMowers(), 0.0f, 6.0f);
                aSoundInstance->AdjustPitch(aPitch);
            }
            aLawnMower->Die();
        }
    }
}

// 0x40C2C0
void Board::CompleteEndLevelSequenceForSaving() {
    if (CanDropLoot()) {
        LawnMower *aLawnMower = nullptr;
        while (IterateLawnMowers(aLawnMower)) {
            if (aLawnMower->mMowerState != LawnMowerState::MOWER_TRIGGERED &&
                aLawnMower->mMowerState != LawnMowerState::MOWER_SQUISHED) {
                const int aCoinValue = Coin::GetCoinValue(CoinType::COIN_GOLD);
                mApp->mPlayerInfo->AddCoins(aCoinValue);
                mBoardData.mCoinsCollected += aCoinValue;
            }
        }
    }

    Coin *aCoin = nullptr;
    while (IterateCoins(aCoin)) {
        if (aCoin->mIsBeingCollected) {
            aCoin->ScoreCoin();
        } else {
            aCoin->Die();
        }
    }

    mApp->UpdatePlayerProfileForFinishingLevel();
}

// 0x40C3E0
void Board::FadeOutLevel() {
    if (mApp->mGameScene != GameScenes::SCENE_PLAYING) {
        RefreshSeedPacketFromCursor();
        mApp->mLastLevelStats->Reset();
        mBoardData.mLevelComplete = true;
    }

    bool aNeedSoundEffect = true;
    if (mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage()) {
        aNeedSoundEffect = false;
    } else if (IsSurvivalStageWithRepick() || IsLastStandStageWithRepick() || mApp->IsEndlessIZombie(mApp->mGameMode)) {
        aNeedSoundEffect = false;
    }
    if (aNeedSoundEffect) {
        mApp->mMusic->StopAllMusic();
        if (mApp->IsAdventureMode() && mBoardData.mLevel == 50) {
            mApp->PlayFoley(FoleyType::FOLEY_FINAL_FANFARE);
        } else if (mApp->TrophiesNeedForGoldSunflower() == 1) {
            mApp->PlayFoley(FoleyType::FOLEY_FINAL_FANFARE);
        } else {
            mApp->PlayFoley(FoleyType::FOLEY_WINMUSIC);
        }
    }

    if (mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage()) {
        mBoardData.mNextSurvivalStageCounter = 500;
        if (mApp->IsAdventureMode()) {
            ClearAdvice(AdviceType::ADVICE_NONE);
        } else {
            mBoardData.mLevelAwardSpawned = true;
            const SexyString aStreakStr = mApp->IsEndlessScaryPotter(mApp->mGameMode) ? _S("[ADVICE_MORE_SCARY_POTS]")
                                                                                      : _S("[ADVICE_3_IN_A_ROW]");
            const SexyString aMessage =
                TodReplaceNumberString(aStreakStr, _S("{STREAK}"), mChallenge->mSurvivalStage + 1);
            PuzzleSaveStreak();
            ClearAdvice(AdviceType::ADVICE_NONE);
            DisplayAdvice(aMessage, MessageStyle::MESSAGE_STYLE_BIG_MIDDLE, AdviceType::ADVICE_NONE);
        }
        return;
    }

    if (mApp->IsEndlessIZombie(mApp->mGameMode)) {
        mBoardData.mNextSurvivalStageCounter = 500;
        const SexyString aMessage =
            TodReplaceNumberString(_S("[ADVICE_MORE_IZOMBIE]"), _S("{STREAK}"), mChallenge->mSurvivalStage + 1);
        PuzzleSaveStreak();
        ClearAdvice(AdviceType::ADVICE_NONE);
        DisplayAdvice(aMessage, MessageStyle::MESSAGE_STYLE_BIG_MIDDLE, AdviceType::ADVICE_NONE);
        return;
    }

    if (IsLastStandStageWithRepick()) {
        mBoardData.mNextSurvivalStageCounter = 500;
        mChallenge->LastStandCompletedStage();
        return;
    }

    if (!IsSurvivalStageWithRepick()) {
        RefreshSeedPacketFromCursor();
        mApp->mLastLevelStats->mUnusedLawnMowers = CountUntriggerLawnMowers();

        mBoardData.mBoardFadeOutCounter = 600;
        if (mBoardData.mLevel == 9 || mBoardData.mLevel == 19 || mBoardData.mLevel == 29 || mBoardData.mLevel == 39 ||
            mBoardData.mLevel == 49) {
            mBoardData.mBoardFadeOutCounter = 500;
        }

        if (CanDropLoot()) {
            mBoardData.mScoreNextMowerCounter = 200;
        }

        Coin *aCoin = nullptr;
        while (IterateCoins(aCoin)) {
            aCoin->TryAutoCollectAfterLevelAward();
        }
    } else {
        TOD_ASSERT(mApp->IsSurvivalMode());
        mBoardData.mNextSurvivalStageCounter = 500;
        DisplayAdvice(_S("[ADVICE_MORE_ZOMBIES]"), MessageStyle::MESSAGE_STYLE_BIG_MIDDLE, AdviceType::ADVICE_NONE);
        mApp->mMusic->FadeOut(500);
        mApp->PlaySample(Sexy::SOUND_HUGE_WAVE);
        for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
            mBoardData.mIceTimer[aRow] = mBoardData.mNextSurvivalStageCounter;
        }
    }

    mApp->SetCursor(CURSOR_POINTER);
}

void Board::DisplayAdvice(
    const SexyString &theAdvice, const MessageStyle theMessageStyle, const AdviceType theHelpIndex
) {
    if (theHelpIndex != AdviceType::ADVICE_NONE) {
        if (IsHelpDisplayed(theHelpIndex)) return;

        IsHelpDisplayed(theHelpIndex) = true;
    }

    mAdvice->SetLabel(theAdvice, theMessageStyle);
    mBoardData.mHelpIndex = theHelpIndex;
}

// 0x40CA10
void Board::DisplayAdviceAgain(
    const SexyString &theAdvice, const MessageStyle theMessageStyle, const AdviceType theHelpIndex
) {
    if (theHelpIndex != AdviceType::ADVICE_NONE) {
        IsHelpDisplayed(theHelpIndex) = false;
    }
    DisplayAdvice(theAdvice, theMessageStyle, theHelpIndex);
}

// 0x40CA50
void Board::ClearAdviceImmediately() {
    ClearAdvice(AdviceType::ADVICE_NONE);
    mAdvice->mDuration = 0;
}

// 0x40CAB0
void Board::ClearAdvice(const AdviceType theHelpIndex) {
    if (theHelpIndex == AdviceType::ADVICE_NONE || theHelpIndex == mBoardData.mHelpIndex) {
        mAdvice->ClearLabel();
        mBoardData.mHelpIndex = AdviceType::ADVICE_NONE;
    }
}

// 0x40CB10
Coin *Board::AddCoin(const int theX, const int theY, const CoinType theCoinType, const CoinMotion theCoinMotion) {
    Coin *aCoin = mCoins.DataArrayAlloc();
    aCoin->CoinInitialize(theX, theY, theCoinType, theCoinMotion);
    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 1) {
        DisplayAdvice(
            _S("[ADVICE_CLICK_ON_SUN]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY,
            AdviceType::ADVICE_CLICK_ON_SUN
        );
    }
    return aCoin;
}

// 0x40CCE0
bool Board::IsPlantInCursor() const {
    return mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK ||
           mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN ||
           mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE ||
           mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_DUPLICATOR ||
           mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW;
}

// 0x40CD10
//  GOTY @Patoke: 0x40F600
SeedType Board::GetSeedTypeInCursor() {
    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WHEEELBARROW) {
        const PottedPlant *aPottedPlant = mApp->mZenGarden->GetPottedPlantInWheelbarrow();
        if (aPottedPlant) {
            return aPottedPlant->mSeedType;
        }
    }

    if (!IsPlantInCursor()) {
        return SeedType::SEED_NONE;
    }
    return mCursorObject->mType == SeedType::SEED_IMITATER ? mCursorObject->mImitaterType : mCursorObject->mType;
}

// 0x40CD80
void Board::RefreshSeedPacketFromCursor() {
    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN) {
        mCoins.DataArrayTryToGet(mCursorObject->mCoinID)->DroppedUsableSeed();
    } else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK) {
        TOD_ASSERT(mCursorObject->mSeedBankIndex >= 0 && mCursorObject->mSeedBankIndex < mSeedBank->mNumPackets);
        mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
    }
    ClearCursor();
}

// 0x40CE00
bool Board::IsPoolSquare(const int theGridX, const int theGridY) const {
    if (theGridX >= 0 && theGridY >= 0) {
        TOD_ASSERT(theGridX < MAX_GRID_SIZE_X && theGridY < MAX_GRID_SIZE_Y);
        return mBoardData.mGridSquareType[theGridX][theGridY] == GridSquareType::GRIDSQUARE_POOL;
    }
    return false;
}

// 0x40CE20
Plant *
Board::NewPlant(const int theGridX, const int theGridY, const SeedType theSeedType, const SeedType theImitaterType) {
    Plant *aPlant = mPlants.DataArrayAlloc();
    aPlant->mIsOnBoard = true;
    aPlant->PlantInitialize(theGridX, theGridY, theSeedType, theImitaterType);
    return aPlant;
}

// 0x40CE60
void Board::DoPlantingEffects(const int theGridX, const int theGridY, const Plant *thePlant) {
    const int aXPos = GridToPixelX(theGridX, theGridY) + 41;
    int aYPos = GridToPixelY(theGridX, theGridY) + 74;
    if (thePlant) {
        if (thePlant->mSeedType == SeedType::SEED_LILYPAD) {
            aYPos += 15;
        } else if (thePlant->mSeedType == SeedType::SEED_FLOWERPOT) {
            aYPos += 30;
        }
    }

    if (mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE) {
        mApp->PlayFoley(FoleyType::FOLEY_CERAMIC);
        return;
    }
    if (mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
        mApp->PlayFoley(FoleyType::FOLEY_PLANT_WATER);
        return;
    }
    if (Plant::IsFlying(thePlant->mSeedType)) {
        mApp->PlayFoley(FoleyType::FOLEY_PLANT);
        return;
    }

    if (IsPoolSquare(theGridX, theGridY)) {
        mApp->PlayFoley(FoleyType::FOLEY_PLANT_WATER);
        mApp->AddTodParticle(aXPos, aYPos, RenderLayer::RENDER_LAYER_TOP, ParticleEffect::PARTICLE_PLANTING_POOL);
    } else {
        mApp->PlayFoley(FoleyType::FOLEY_PLANT);
        mApp->AddTodParticle(aXPos, aYPos, RenderLayer::RENDER_LAYER_TOP, ParticleEffect::PARTICLE_PLANTING);
    }
}

// 0x40D120
//  GOTY @Patoke: 0x40FA10
Plant *
Board::AddPlant(const int theGridX, const int theGridY, const SeedType theSeedType, const SeedType theImitaterType) {
    Plant *aPlant = NewPlant(theGridX, theGridY, theSeedType, theImitaterType);
    DoPlantingEffects(theGridX, theGridY, aPlant);
    mChallenge->PlantAdded(aPlant);

    const int aSunPlantsCount = CountPlantByType(SeedType::SEED_SUNSHROOM) + CountPlantByType(SeedType::SEED_SUNFLOWER);
    if (aSunPlantsCount > mBoardData.mMaxSunPlants) {
        mBoardData.mMaxSunPlants = aSunPlantsCount;
        // mMaxSunPlants = max(aSunPlantsCount, mMaxSunPlants);
    }

    // @Patoke: implemented
    if (theSeedType == SeedType::SEED_PEASHOOTER || theSeedType == SeedType::SEED_SNOWPEA ||
        theSeedType == SeedType::SEED_REPEATER || theSeedType == SeedType::SEED_THREEPEATER ||
        theSeedType == SeedType::SEED_SPLITPEA || theSeedType == SeedType::SEED_GATLINGPEA) {
        mBoardData.mPeaShooterUsed = true;
    }
    if (theSeedType == SeedType::SEED_CABBAGEPULT || theSeedType == SeedType::SEED_KERNELPULT ||
        theSeedType == SeedType::SEED_MELONPULT || theSeedType == SeedType::SEED_WINTERMELON) {
        mBoardData.mCatapultPlantsUsed = true;
    }

    const bool aIsFungi = Plant::IsFungus(theSeedType);
    if (!Plant::IsFlying(theSeedType) && !aIsFungi) {
        mBoardData.mMushroomAndCoffeeBeansOnly = false;
    }
    if (aIsFungi) {
        mBoardData.mMushroomsUsed = true;
    }

    return aPlant;
}

// 0x40D1A0
//  GOTY @Patoke: 0x40FBA0
Plant *Board::GetPumpkinAt(const int theGridX, const int theGridY) {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY && !aPlant->NotOnGround() &&
            aPlant->mSeedType == SeedType::SEED_PUMPKINSHELL) {
            return aPlant;
        }
    }
    return nullptr;
}

// 0x40D220
Plant *Board::GetFlowerPotAt(const int theGridX, const int theGridY) {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY && !aPlant->NotOnGround() &&
            aPlant->mSeedType == SeedType::SEED_FLOWERPOT) {
            return aPlant;
        }
    }
    return nullptr;
}

// 0x40D2A0
void Board::GetPlantsOnLawn(const int theGridX, const int theGridY, PlantsOnLawn *thePlantOnLawn) {
    thePlantOnLawn->mUnderPlant = nullptr;
    thePlantOnLawn->mPumpkinPlant = nullptr;
    thePlantOnLawn->mFlyingPlant = nullptr;
    thePlantOnLawn->mNormalPlant = nullptr;

    if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y) return;

    if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial()) return;

    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        SeedType aSeedType = aPlant->mSeedType;
        if (aSeedType == SeedType::SEED_IMITATER && aPlant->mImitaterType != SeedType::SEED_NONE) {
            aSeedType = aPlant->mImitaterType;
        }

        // 检测植物是否位于目标格子内
        if (aPlant->mRow != theGridY) {
            continue;
        }
        if (aSeedType == SeedType::SEED_COBCANNON) {
            if (aPlant->mPlantCol < theGridX - 1 || aPlant->mPlantCol > theGridX) {
                continue;
            }
        } else {
            if (aPlant->mPlantCol != theGridX) {
                continue;
            }
        }
        if (aPlant->NotOnGround()) {
            continue;
        }

        // 将植物写入 thePlantOnLawn 的记录
        if (Plant::IsFlying(aPlant->mSeedType)) {
            TOD_ASSERT(!thePlantOnLawn->mFlyingPlant);
            thePlantOnLawn->mFlyingPlant = aPlant;
        } else if (aSeedType == SeedType::SEED_FLOWERPOT || (aSeedType == SeedType::SEED_LILYPAD && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)) {
            TOD_ASSERT(!thePlantOnLawn->mUnderPlant);
            thePlantOnLawn->mUnderPlant = aPlant;
        } else if (aSeedType == SeedType::SEED_PUMPKINSHELL) {
            TOD_ASSERT(!thePlantOnLawn->mPumpkinPlant);
            thePlantOnLawn->mPumpkinPlant = aPlant;
        } else {
            TOD_ASSERT(!thePlantOnLawn->mNormalPlant);
            thePlantOnLawn->mNormalPlant = aPlant;
        }
    }
}

Plant *Board::GetTopPlantAt(const int theGridX, const int theGridY, const PlantPriority thePriority) {
    if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y) return nullptr;

    if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial()) return nullptr;

    PlantsOnLawn aPlantOnLawn;
    GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);

    switch (thePriority) {
    case PlantPriority::TOPPLANT_EATING_ORDER:
        if (aPlantOnLawn.mPumpkinPlant) return aPlantOnLawn.mPumpkinPlant;
        else if (aPlantOnLawn.mNormalPlant) return aPlantOnLawn.mNormalPlant;
        else return aPlantOnLawn.mUnderPlant;
    case PlantPriority::TOPPLANT_DIGGING_ORDER:
        if (aPlantOnLawn.mNormalPlant) return aPlantOnLawn.mNormalPlant;
        else return aPlantOnLawn.mUnderPlant;
    case PlantPriority::TOPPLANT_BUNGEE_ORDER:
    case PlantPriority::TOPPLANT_CATAPULT_ORDER:
    case PlantPriority::TOPPLANT_ANY:
        if (aPlantOnLawn.mFlyingPlant) return aPlantOnLawn.mFlyingPlant;
        else if (aPlantOnLawn.mNormalPlant) return aPlantOnLawn.mNormalPlant;
        else if (aPlantOnLawn.mPumpkinPlant) return aPlantOnLawn.mPumpkinPlant;
        else return aPlantOnLawn.mUnderPlant;
    case PlantPriority::TOPPLANT_ZEN_TOOL_ORDER:
        if (aPlantOnLawn.mFlyingPlant) return aPlantOnLawn.mFlyingPlant;
        else if (aPlantOnLawn.mPumpkinPlant) return aPlantOnLawn.mPumpkinPlant;
        else if (aPlantOnLawn.mNormalPlant) return aPlantOnLawn.mNormalPlant;
        else return aPlantOnLawn.mUnderPlant;
    case PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION: return aPlantOnLawn.mNormalPlant;
    case PlantPriority::TOPPLANT_ONLY_FLYING:          return aPlantOnLawn.mFlyingPlant;
    case PlantPriority::TOPPLANT_ONLY_PUMPKIN:         return aPlantOnLawn.mPumpkinPlant;
    case PlantPriority::TOPPLANT_ONLY_UNDER_PLANT:     return aPlantOnLawn.mUnderPlant;
    default:                                           TOD_ASSERT();
    }
    unreachable();
}

// 0x40D3A0
int Board::CountSunFlowers() {
    int aCount = 0;
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->MakesSun()) {
            aCount++;
        }
    }
    return aCount;
}

// 0x40D3F0
int Board::CountPlantByType(const SeedType theSeedType) {
    int aCount = 0;
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mSeedType == theSeedType) {
            aCount++;
        }
    }
    return aCount;
}

// 0x40D430
int Board::CountEmptyPotsOrLilies(const SeedType theSeedType) {
    int aCount = 0;
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mSeedType == theSeedType &&
            !GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION)) {
            aCount++;
        }
    }
    return aCount;
}

// 0x40D4F0
bool Board::IsValidCobCannonSpotHelper(const int theGridX, const int theGridY) {
    PlantsOnLawn aPlantOnLawn;
    GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);
    if (aPlantOnLawn.mPumpkinPlant) return false;

    if (aPlantOnLawn.mNormalPlant && aPlantOnLawn.mNormalPlant->mSeedType == SeedType::SEED_KERNELPULT) return true;

    return mApp->mEasyPlantingCheat &&
           CanPlantAt(theGridX, theGridY, SeedType::SEED_KERNELPULT) == PlantingReason::PLANTING_OK;
}

// 0x40D550
bool Board::IsValidCobCannonSpot(const int theGridX, const int theGridY) {
    if (!IsValidCobCannonSpotHelper(theGridX, theGridY) || !IsValidCobCannonSpotHelper(theGridX + 1, theGridY))
        return false;

    return !GetFlowerPotAt(theGridX, theGridY) == !GetFlowerPotAt(theGridX + 1, theGridY);
}

// 0x40D5C0
bool Board::HasValidCobCannonSpot() {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mSeedType == SeedType::SEED_KERNELPULT && IsValidCobCannonSpot(aPlant->mPlantCol, aPlant->mRow)) {
            return true;
        }
    }
    return false;
}

// 0x40D620
Projectile *Board::AddProjectile(
    const int theX, const int theY, const int theRenderOrder, const int theRow, const ProjectileType theProjectileType
) {
    Projectile *aProjectile = mProjectiles.DataArrayAlloc();
    aProjectile->ProjectileInitialize(theX, theY, theRenderOrder, theRow, theProjectileType);
    return aProjectile;
}

// 0x40D660
bool Board::CanZombieSpawnOnLevel(const ZombieType theZombieType, const int theLevel) {
    const ZombieDefinition &aZombieDef = GetZombieDefinition(theZombieType);
    if (theZombieType == ZombieType::ZOMBIE_YETI) {
        return gLawnApp->CanSpawnYetis();
    }

    if (theLevel < aZombieDef.mStartingLevel || aZombieDef.mPickWeight == 0) {
        return false;
    }

    TOD_ASSERT(gZombieAllowedLevels[theZombieType].mZombieType == theZombieType);
    return gZombieAllowedLevels[theZombieType].mAllowedOnLevel[ClampInt(theLevel - 1, 0, 49)];
}

// 0x40D6F0
ZombieType Board::GetIntroducedZombieType() const {
    if (!mApp->IsAdventureMode() || mBoardData.mLevel == 1) {
        return ZombieType::ZOMBIE_INVALID;
    }

    for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES;
         aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1)) {
        const ZombieDefinition &aZombieDef = GetZombieDefinition(aZombieType);
        if ((aZombieType != ZombieType::ZOMBIE_YETI || mApp->CanSpawnYetis()) &&
            aZombieDef.mStartingLevel == mBoardData.mLevel) {
            return aZombieType;
        }
    }
    return ZombieType::ZOMBIE_INVALID;
}

// 0x40D770
ZombieType Board::PickGraveRisingZombieType() {
    TodWeightedArray<ZombieType> aZombieWeightArray[ZombieType::NUM_ZOMBIE_TYPES];
    int aCount = 2;
    aZombieWeightArray[0].mItem = ZombieType::ZOMBIE_NORMAL;
    aZombieWeightArray[0].mWeight = GetZombieDefinition(ZombieType::ZOMBIE_NORMAL).mPickWeight;
    aZombieWeightArray[1].mItem = ZombieType::ZOMBIE_TRAFFIC_CONE;
    aZombieWeightArray[1].mWeight = GetZombieDefinition(ZombieType::ZOMBIE_TRAFFIC_CONE).mPickWeight;
    if (!StageHasGraveStones()) {
        aZombieWeightArray[2].mItem = ZombieType::ZOMBIE_PAIL;
        aZombieWeightArray[2].mWeight = GetZombieDefinition(ZombieType::ZOMBIE_PAIL).mPickWeight;
        aCount++;
    }

    for (int i = 0; i < aCount; i++) {
        const ZombieType aZombieType = aZombieWeightArray[i].mItem;
        const ZombieDefinition &aZombieDef = GetZombieDefinition(aZombieType);
        if ((mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel < aZombieDef.mStartingLevel) ||
            (!mBoardData.mZombieAllowed[aZombieType] && aZombieType != ZombieType::ZOMBIE_NORMAL)) {
            aZombieWeightArray[i].mWeight = 0;
        }
    }

    return TodPickFromWeightedArray(aZombieWeightArray, aCount);
}

// 0x40D8A0
ZombieType
Board::PickZombieType(const int theZombiePoints, const int theWaveIndex, const ZombiePicker *theZombiePicker) {
    int aPickCount = 0;
    TodWeightedArray<ZombieType> aZombieWeightArray[ZombieType::NUM_ZOMBIE_TYPES];
    for (int aZombieTypeInt = ZombieType::ZOMBIE_NORMAL; aZombieTypeInt < ZombieType::NUM_ZOMBIE_TYPES;
         aZombieTypeInt++) {
        const auto aZombieType = static_cast<ZombieType>(aZombieTypeInt);
        if (!mBoardData.mZombieAllowed[aZombieType]) continue;

        const ZombieDefinition &aZombieDef = GetZombieDefinition((ZombieType)aZombieType);

        // ================================================================================================
        // ▲ 将不符合出怪限制或超出剩余点数的僵尸类型排除
        // ================================================================================================
        const GameMode aGameMode = mApp->mGameMode;
        // 蹦极僵尸在无尽模式中仅在旗帜波出现
        if (aZombieType == ZombieType::ZOMBIE_BUNGEE && mApp->IsSurvivalEndless(aGameMode)) {
            if (!IsFlagWave(theWaveIndex)) {
                continue;
            }
        }
        // 僵尸最早出现的波数的限制（出怪限制）
        else if (aGameMode != GameMode::GAMEMODE_CHALLENGE_POGO_PARTY && aGameMode != GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA && aGameMode != GameMode::GAMEMODE_CHALLENGE_AIR_RAID) {
            int aFirstAllowedWave = aZombieDef.mFirstAllowedWave;
            // 无尽模式中，僵尸最早可出现的波数逐渐前移
            if (mApp->IsSurvivalEndless(aGameMode)) {
                const int aFlags = GetSurvivalFlagsCompleted();
                int aAllowedWave = aFirstAllowedWave - TodAnimateCurve(18, 50, aFlags, 0, 15, TodCurves::CURVE_LINEAR);
                aFirstAllowedWave = std::max(aAllowedWave, 1);
            }
            if (theWaveIndex + 1 < aFirstAllowedWave || theZombiePoints < aZombieDef.mZombieValue) {
                continue;
            }
        }

        // ================================================================================================
        // ▲ 生存模式中，根据当前旗帜数等重新计算僵尸的权重
        // ================================================================================================
        int aPickWeight = aZombieDef.mPickWeight;
        if (mApp->IsSurvivalMode()) {
            const int aFlags = GetSurvivalFlagsCompleted();
            // 伽刚特尔和雪橇车僵尸的每波出怪上限
            if (aZombieType == ZombieType::ZOMBIE_GARGANTUAR || aZombieType == ZombieType::ZOMBIE_ZAMBONI) {
                if (theZombiePicker->mZombieTypeCount[aZombieType] >=
                    TodAnimateCurve(10, 50, aFlags, 2, 50, TodCurves::CURVE_LINEAR)) {
                    continue;
                }
            }
            // 红眼的旗帜波出怪上限和非旗帜波出怪总和上限
            else if (aZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR) {
                if (IsFlagWave(theWaveIndex)) {
                    if (theZombiePicker->mZombieTypeCount[aZombieType] >=
                        TodAnimateCurve(14, 100, aFlags, 1, 50, TodCurves::CURVE_LINEAR)) {
                        continue;
                    }
                } else {
                    if (theZombiePicker->mAllWavesZombieTypeCount[aZombieType] >=
                        TodAnimateCurve(10, 110, aFlags, 1, 50, TodCurves::CURVE_LINEAR)) {
                        continue;
                    }
                    aPickWeight = 1000;
                }
            }
            // 普通僵尸和路障僵尸的权重衰减
            else if (aZombieType == ZombieType::ZOMBIE_NORMAL) {
                aPickWeight = TodAnimateCurve(10, 50, aFlags, aPickWeight, aPickWeight / 10, TodCurves::CURVE_LINEAR);
            } else if (aZombieType == ZombieType::ZOMBIE_TRAFFIC_CONE) {
                aPickWeight = TodAnimateCurve(10, 50, aFlags, aPickWeight, aPickWeight / 4, TodCurves::CURVE_LINEAR);
            }
        }
        aZombieWeightArray[aPickCount].mItem = aZombieType;
        aZombieWeightArray[aPickCount].mWeight = aPickWeight;
        aPickCount++;
    }

    // 加权随机地取得一种可能的僵尸类型并返回
    return (ZombieType)TodPickFromWeightedArray(aZombieWeightArray, aPickCount);
}

bool Board::IsZombieTypePoolOnly(const ZombieType theZombieType) {
    return (theZombieType == ZombieType::ZOMBIE_SNORKEL || theZombieType == ZombieType::ZOMBIE_DOLPHIN_RIDER);
}

// 0x40DB20
bool Board::RowCanHaveZombieType(const int theRow, const ZombieType theZombieType) {
    if (!RowCanHaveZombies(theRow)) {
        return false;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED &&
        mBoardData.mPlantRow[theRow] == PlantRowType::PLANTROW_DIRT && mBoardData.mCurrentWave < 5) {
        return false; // 无草皮之地关卡，无草皮的行在前 5 波不刷出僵尸
    }
    if (mBoardData.mPlantRow[theRow] == PlantRowType::PLANTROW_POOL && !Zombie::ZombieTypeCanGoInPool(theZombieType)) {
        return false; // 水路不会刷出不能进入泳池的僵尸
    }
    if (mBoardData.mPlantRow[theRow] == PlantRowType::PLANTROW_HIGH_GROUND &&
        !Zombie::ZombieTypeCanGoOnHighGround(theZombieType)) {
        return false; // 高地不会刷出不能走上高地的僵尸
    }

    int aCurrentWave = mBoardData.mCurrentWave;
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
        aCurrentWave += mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage();
    }
    // 非水路不能刷出水路僵尸；前 5 小波，水面仅刷出潜水僵尸或海豚骑士僵尸
    if (mBoardData.mPlantRow[theRow] == PlantRowType::PLANTROW_POOL) {
        if (aCurrentWave < 5 && !IsZombieTypePoolOnly(theZombieType)) {
            return false;
        }
    } else if (IsZombieTypePoolOnly(theZombieType)) {
        return false;
    }
    // 雪橇僵尸小队仅能在有冰道的行刷出
    if (theZombieType == ZOMBIE_BOBSLED && !mBoardData.mIceTimer[theRow]) {
        return false;
    }
    // “自古一路无巨人”（生存模式除外）
    if (theRow == 0 && !mApp->IsSurvivalMode()) {
        if (theZombieType == ZombieType::ZOMBIE_GARGANTUAR || theZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR) {
            return false;
        }
    }
    // 非舞王僵尸或当前为泳池关卡，则可以刷出该僵尸
    if (theZombieType != ZombieType::ZOMBIE_DANCER || StageHasPool()) {
        return true;
    }
    // 舞王僵尸在非泳池关卡中，为保证能召唤伴舞僵尸，仅在中间三行刷出
    return RowCanHaveZombies(theRow - 1) && RowCanHaveZombies(theRow + 1);
}

// 0x40DC50
int Board::PickRowForNewZombie(const ZombieType theZombieType) {
    // ====================================================================================================
    // ▲ 当存在正在寻找目标僵尸的钉耙，且僵尸可以出现在钉耙所在行时，优先出现在钉耙所在行
    // ====================================================================================================
    GridItem *aRake = GetRake();
    if (aRake && aRake->mGridItemState == GridItemState::GRIDITEM_STATE_RAKE_ATTRACTING &&
        RowCanHaveZombieType(aRake->mGridY, theZombieType)) {
        aRake->mGridItemState = GridItemState::GRIDITEM_STATE_RAKE_WAITING;
        TodUpdateSmoothArrayPick(mBoardData.mRowPickingArray, MAX_GRID_SIZE_Y, aRake->mGridY);
        return aRake->mGridY;
    }

    // ====================================================================================================
    // ▲ 遍历每一行，将所有能允许该僵尸出现的行及其对应权重写入挑选数组中
    // ====================================================================================================
    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        // 如果本行不能出现目标僵尸，则将本行权重置零，并继续下一行
        if (!RowCanHaveZombieType(aRow, theZombieType)) {
            mBoardData.mRowPickingArray[aRow].mWeight = 0;
        }
        // 保护传送门关卡中，每行的出怪概率受传送门位置影响
        else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT) {
            mBoardData.mRowPickingArray[aRow].mWeight = mChallenge->PortalCombatRowSpawnWeight(aRow);
        }
        // 隐形食脑者关卡中，前 3 波第六路不出怪
        else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL && mBoardData.mCurrentWave <= 3 && aRow == 5) {
            mBoardData.mRowPickingArray[aRow].mWeight = 0;
        }
        // 丢车保护
        else {
            int aWavesMowered = mBoardData.mCurrentWave - mBoardData.mWaveRowGotLawnMowered[aRow];
            if (mApp->IsContinuousChallenge() && mBoardData.mCurrentWave == mBoardData.mNumWaves - 1) {
                aWavesMowered = 100;
            }

            if (aWavesMowered <= 1) {
                mBoardData.mRowPickingArray[aRow].mWeight = 0.01f;
            } else if (aWavesMowered <= 2) {
                mBoardData.mRowPickingArray[aRow].mWeight = 0.5f;
            } else {
                mBoardData.mRowPickingArray[aRow].mWeight = 1.0f;
            }
        }
    }
    return TodPickFromSmoothArray(mBoardData.mRowPickingArray, MAX_GRID_SIZE_Y);
}

// 0x40DD90
bool Board::CanAddBobSled() const {
    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        if (mBoardData.mIceTimer[aRow] > 0 && mBoardData.mIceMinX[aRow] < 700) {
            return true;
        }
    }
    return false;
}

// 0x40DDC0
//  GOTY @Patoke: 0x410700
Zombie *Board::AddZombieInRow(const ZombieType theZombieType, const int theRow, const int theFromWave) {
    if (mZombies.mSize >= mZombies.mMaxSize - 1) {
        fmt::println("Too many zombies!!");
        return nullptr;
    }

    // @Patoke: implemented
    if (theZombieType == ZombieType::ZOMBIE_YETI) {
        if (mApp->IsAdventureMode() && mBoardData.mLevel == 40 && theFromWave >= 0)
            ReportAchievement::GiveAchievement(mApp, Zombologist, true);
    }

    const bool aVariant = !Rand(5);
    Zombie *aZombie = mZombies.DataArrayAlloc();
    aZombie->ZombieInitialize(theRow, theZombieType, aVariant, nullptr, theFromWave);
    if (theZombieType == ZombieType::ZOMBIE_BOBSLED && aZombie->IsOnBoard()) {
        for (int _i = 0; _i < 3; _i++) {
            mZombies.DataArrayAlloc()->ZombieInitialize(
                theRow, ZombieType::ZOMBIE_BOBSLED, false, aZombie, theFromWave
            );
        }
    }
    return aZombie;
}

Zombie *Board::AddZombie(const ZombieType theZombieType, const int theFromWave) {
    return AddZombieInRow(theZombieType, PickRowForNewZombie(theZombieType), theFromWave);
}

// 0x40DEA0
void Board::RemoveAllZombies() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (!aZombie->IsDeadOrDying()) {
            aZombie->DieNoLoot();
        }
    }
}

// 0x40DF00
void Board::RemoveZombiesForRepick() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (!aZombie->IsDeadOrDying() && aZombie->mMindControlled && aZombie->mPosX > 720) {
            aZombie->DieNoLoot();
        }
    }
}

// 0x40DF70
void Board::RemoveCutsceneZombies() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mFromWave == Zombie::ZOMBIE_WAVE_CUTSCENE) {
            aZombie->DieNoLoot();
        }
    }
}

// 0x40DFC0
bool Board::IsIceAt(const int theGridX, const int theGridY) {
    TOD_ASSERT(theGridY >= 0 && theGridY < MAX_GRID_SIZE_Y);
    if (mBoardData.mIceTimer[theGridY] == 0 || mBoardData.mIceMinX[theGridY] > 750) return false;

    return theGridX >= PixelToGridXKeepOnBoard(mBoardData.mIceMinX[theGridY] + 12, 0);
}

// 0x40E020
PlantingReason Board::CanPlantAt(const int theGridX, const int theGridY, const SeedType theSeedType) {
    // 目标位置不在场地内，则返回“不能种在那里”
    if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y) {
        return PlantingReason::PLANTING_NOT_HERE;
    }

    // 从关卡玩法上，判断能否种植
    const PlantingReason aReason = mChallenge->CanPlantAt(theGridX, theGridY, theSeedType);
    if (aReason != PlantingReason::PLANTING_OK || Challenge::IsZombieSeedType(theSeedType)) {
        return aReason;
    }

    PlantsOnLawn aPlantOnLawn;
    GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (aPlantOnLawn.mUnderPlant || aPlantOnLawn.mPumpkinPlant || aPlantOnLawn.mFlyingPlant ||
            aPlantOnLawn.mNormalPlant) {
            return PlantingReason::PLANTING_NOT_HERE;
        }
        if (mApp->mZenGarden->mGardenType == GardenType::GARDEN_AQUARIUM && !Plant::IsAquatic(theSeedType)) {
            return PlantingReason::PLANTING_NOT_ON_WATER;
        }

        return PlantingReason::PLANTING_OK;
    }

    // 墓碑吞噬者只能种植在墓碑上
    const bool aHasGrave = GetGraveStoneAt(theGridX, theGridY);
    if (theSeedType == SeedType::SEED_GRAVEBUSTER) {
        if (aPlantOnLawn.mNormalPlant) {
            return PlantingReason::PLANTING_NOT_HERE;
        }

        return aHasGrave ? PlantingReason::PLANTING_OK : PlantingReason::PLANTING_ONLY_ON_GRAVES;
    }
    if (theSeedType == SeedType::SEED_INSTANT_COFFEE) {
        if (aPlantOnLawn.mFlyingPlant) {
            return PlantingReason::PLANTING_NOT_HERE;
        }

        if (!aPlantOnLawn.mNormalPlant || !aPlantOnLawn.mNormalPlant->mIsAsleep ||
            aPlantOnLawn.mNormalPlant->mWakeUpCounter > 0 ||
            aPlantOnLawn.mNormalPlant->mOnBungeeState == PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE) {
            return PlantingReason::PLANTING_NEEDS_SLEEPING;
        }

        return PlantingReason::PLANTING_OK;
    }
    // 非墓碑吞噬者且非飞行植物，则不能种在墓碑上
    if (aHasGrave) {
        return Plant::IsFlying(theSeedType) ? PlantingReason::PLANTING_OK : PlantingReason::PLANTING_NOT_ON_GRAVE;
    }

    Plant *aUnderPlant = aPlantOnLawn.mUnderPlant;
    bool aHasLilypad, aHasFlowerPot;
    if (!aUnderPlant || aUnderPlant->mOnBungeeState == PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE) {
        aHasLilypad = false;
        aHasFlowerPot = false;
    } else {
        aHasLilypad = aUnderPlant->mSeedType == SeedType::SEED_LILYPAD;
        aHasFlowerPot = aUnderPlant->mSeedType == SeedType::SEED_FLOWERPOT;
    }
    // 部分情况下的格子中不能种植植物
    if (GetCraterAt(theGridX, theGridY)) {
        return PlantingReason::PLANTING_NOT_ON_CRATER;
    }
    if (GetScaryPotAt(theGridX, theGridY) || IsIceAt(theGridX, theGridY)) {
        return PlantingReason::PLANTING_NOT_HERE;
    }
    const GridSquareType aGridSquare = mBoardData.mGridSquareType[theGridX][theGridY];
    if (aGridSquare == GridSquareType::GRIDSQUARE_DIRT || aGridSquare == GridSquareType::GRIDSQUARE_NONE) {
        return PlantingReason::PLANTING_NOT_HERE;
    }
    // 水生植物只能种在水上
    Plant *aNormalPlant = aPlantOnLawn.mNormalPlant;
    if (theSeedType == SeedType::SEED_LILYPAD || theSeedType == SeedType::SEED_TANGLEKELP ||
        theSeedType == SeedType::SEED_SEASHROOM) {
        if (!IsPoolSquare(theGridX, theGridY)) {
            return PlantingReason::PLANTING_ONLY_IN_POOL;
        }

        return (aNormalPlant || aUnderPlant) ? PlantingReason::PLANTING_NOT_HERE : PlantingReason::PLANTING_OK;
    }
    if (Plant::IsFlying(theSeedType)) {
        return aPlantOnLawn.mFlyingPlant ? PlantingReason::PLANTING_NOT_HERE : PlantingReason::PLANTING_OK;
    }
    // 地刺/地刺王只能种在坚固的地面
    if (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK) {
        if (aGridSquare == GridSquareType::GRIDSQUARE_POOL || StageHasRoof() || aUnderPlant) {
            return PlantingReason::PLANTING_NEEDS_GROUND;
        }
    }
    // 非水生植物不能种在水面上（南瓜头可以种在香蒲上）
    const Plant *aPumpkinPlant = aPlantOnLawn.mPumpkinPlant;
    if (aGridSquare == GridSquareType::GRIDSQUARE_POOL && !aHasLilypad && theSeedType != SeedType::SEED_CATTAIL) {
        if (!aNormalPlant || aNormalPlant->mSeedType != SeedType::SEED_CATTAIL ||
            theSeedType != SeedType::SEED_PUMPKINSHELL) {
            return PlantingReason::PLANTING_NOT_ON_WATER;
        }
    }
    // 花盆的种植条件
    if (theSeedType == SeedType::SEED_FLOWERPOT) {
        return (aNormalPlant || aUnderPlant || aPumpkinPlant) ? PlantingReason::PLANTING_NOT_HERE
                                                              : PlantingReason::PLANTING_OK;
    }
    // 屋顶种植需要花盆
    if (StageHasRoof() && !aHasFlowerPot) {
        return PlantingReason::PLANTING_NEEDS_POT;
    }
    // 南瓜头的种植条件
    const bool aAidPurchased = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FIRSTAID] > 0;
    if (theSeedType == SeedType::SEED_PUMPKINSHELL) {
        // 不可种植在玉米加农炮上
        if (aNormalPlant && aNormalPlant->mSeedType == SeedType::SEED_COBCANNON) {
            return PlantingReason::PLANTING_NOT_HERE;
        }
        // 无南瓜头时，可以种植南瓜头
        if (!aPumpkinPlant) {
            return PlantingReason::PLANTING_OK;
        }
        // 南瓜头的坚果包扎术
        if (aAidPurchased && aPumpkinPlant->mPlantHealth < aPumpkinPlant->mPlantMaxHealth * 2 / 3 &&
            aPumpkinPlant->mSeedType == SeedType::SEED_PUMPKINSHELL &&
            aPumpkinPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE) {
            return PlantingReason::PLANTING_OK;
        }

        return PlantingReason::PLANTING_NOT_HERE;
    }
    // 土豆地雷只能种在陆地上
    if (aHasLilypad && theSeedType == SeedType::SEED_POTATOMINE) {
        return PlantingReason::PLANTING_ONLY_ON_GROUND;
    }

    if (aUnderPlant) {
        // 香蒲对底端植物的紫卡升级
        if (theSeedType == SeedType::SEED_CATTAIL) {
            if (aNormalPlant) {
                return PlantingReason::PLANTING_NOT_HERE;
            }
            if (aUnderPlant->IsUpgradableTo(theSeedType) &&
                aUnderPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE) {
                return PlantingReason::PLANTING_OK;
            }
            if (Plant::IsUpgrade(theSeedType)) {
                return PlantingReason::PLANTING_NEEDS_UPGRADE;
            }
        } else {
            // 模仿中的模仿者不可作为花盆或睡莲
            if (aUnderPlant->mSeedType == SeedType::SEED_IMITATER) {
                return PlantingReason::PLANTING_NOT_HERE;
            }
        }
    }

    // 一般紫卡植物的更迭判断
    if (aNormalPlant) {
        // 紫卡植物的升级
        if (aNormalPlant->IsUpgradableTo(theSeedType) &&
            aNormalPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE) {
            return PlantingReason::PLANTING_OK;
        }
        if (Plant::IsUpgrade(theSeedType)) {
            return PlantingReason::PLANTING_NEEDS_UPGRADE;
        }

        // 坚果包扎术
        if ((theSeedType == SeedType::SEED_WALLNUT || theSeedType == SeedType::SEED_TALLNUT) && aAidPurchased) {
            if (aNormalPlant->mPlantHealth < aNormalPlant->mPlantMaxHealth * 2 / 3 &&
                aNormalPlant->mSeedType == theSeedType &&
                aNormalPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE) {
                return PlantingReason::PLANTING_OK;
            }
        }

        return PlantingReason::PLANTING_NOT_HERE;
    }

    // 免费种植模式下紫卡的额外判断
    if (!mApp->mEasyPlantingCheat && Plant::IsUpgrade(theSeedType)) {
        return PlantingReason::PLANTING_NEEDS_UPGRADE;
    }
    if (theSeedType == SeedType::SEED_COBCANNON && !IsValidCobCannonSpot(theGridX, theGridY)) {
        return PlantingReason::PLANTING_NEEDS_UPGRADE;
    }
    if (theSeedType == SeedType::SEED_CATTAIL && aGridSquare != GridSquareType::GRIDSQUARE_POOL) {
        return PlantingReason::PLANTING_NOT_HERE;
    }

    return PlantingReason::PLANTING_OK;
}

// 0x40E520
void Board::UpdateCursor() {
    const int aMouseX = mApp->mWidgetManager->mLastMouseX - mX;
    const int aMouseY = mApp->mWidgetManager->mLastMouseY - mY;
    bool aShowFinger = false;
    constexpr bool aShowDrag = false;
    bool aHideCursor = false;

    if (mApp->mSeedChooserScreen && mApp->mSeedChooserScreen->Contains(aMouseX + mX, aMouseY + mY)) return;

    if (mApp->GetDialogCount() > 0) return;

    if (mBoardData.mPaused || mBoardData.mBoardFadeOutCounter >= 0 || mBoardData.mTimeStopCounter > 0 ||
        mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) {
        mApp->SetCursor(CURSOR_POINTER);
        return;
    }

    HitResult aHitResult;
    MouseHitTest(aMouseX, aMouseY, &aHitResult);
    switch (aHitResult.mObjectType) {
    case GameObjectType::OBJECT_TYPE_MENU_BUTTON:
    case GameObjectType::OBJECT_TYPE_STORE_BUTTON:
    case GameObjectType::OBJECT_TYPE_SHOVEL:
    case GameObjectType::OBJECT_TYPE_WATERING_CAN:
    case GameObjectType::OBJECT_TYPE_FERTILIZER:
    case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
    case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
    case GameObjectType::OBJECT_TYPE_CHOCOLATE:
    case GameObjectType::OBJECT_TYPE_GLOVE:
    case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
    case GameObjectType::OBJECT_TYPE_NEXT_GARDEN:
    case GameObjectType::OBJECT_TYPE_WHEELBARROW:
    case GameObjectType::OBJECT_TYPE_SLOT_MACHINE_HANDLE:
    case GameObjectType::OBJECT_TYPE_TREE_FOOD:
    case GameObjectType::OBJECT_TYPE_STINKY:
    case GameObjectType::OBJECT_TYPE_TREE_OF_WISDOM:
    case GameObjectType::OBJECT_TYPE_COIN:
    case GameObjectType::OBJECT_TYPE_PROJECTILE:          aShowFinger = true; break;

    case GameObjectType::OBJECT_TYPE_SEEDPACKET:
        aShowFinger = static_cast<SeedPacket *>(aHitResult.mObject)->CanPickUp();
        break;

    case GameObjectType::OBJECT_TYPE_SCARY_POT:
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL) {
            aShowFinger = true;
        } else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER) {
            aHideCursor = true;
        }
        break;

    case GameObjectType::OBJECT_TYPE_PLANT:
        if ((mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
             mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST) &&
            !HasLevelAwardDropped()) {
            aShowFinger = true;
        }
        if (static_cast<Plant *>(aHitResult.mObject)->mState == PlantState::STATE_COBCANNON_READY) {
            aShowFinger = true;
        }
        break;

    default:
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER) {
            aHideCursor = true;
        }
        break;
    }

    if (mChallenge->mBeghouledMouseCapture || aShowDrag) {
        mApp->SetCursor(CURSOR_DRAGGING);
    } else if (aShowFinger) {
        mApp->SetCursor(CURSOR_HAND);
    } else if (aHideCursor) {
        mApp->SetCursor(CURSOR_NONE);
    } else {
        mApp->SetCursor(CURSOR_POINTER);
    }
}

// 0x40E760
void Board::MouseMove(const int x, const int y) {
    Widget::MouseMove(x, y);
    mChallenge->MouseMove(x, y);
}

// 0x40E760
void Board::MouseDrag(const int x, const int y) {
    Widget::MouseDrag(x, y);
    mChallenge->MouseMove(x, y);
}

// 0x40E780
Zombie *Board::ZombieHitTest(const int theMouseX, const int theMouseY) {
    Zombie *aZombie = nullptr;
    Zombie *aRecord = nullptr;
    while (IterateZombies(aZombie)) {
        // 排除已死亡的僵尸
        if (aZombie->mDead || aZombie->IsDeadOrDying()) continue;

        // 排除关卡引入阶段及选卡界面的植物僵尸
        if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && Zombie::IsZombotany(aZombie->mZombieType)) continue;

        // 范围判定
        if (aZombie->GetZombieRect().Contains(theMouseX, theMouseY)) {
            if (aRecord == nullptr || aZombie->mY > aRecord->mY) {
                aRecord = aZombie;
            }
        }
    }

    return aRecord;
}

// 0x40E880
bool Board::IsPlantInGoldWateringCanRange(const int theMouseX, const int theMouseY, const Plant *thePlant) {
    /*
    Rect aRect = Rect(theMouseX - 70, theMouseY - 80, 160, 160);
    if (GetTopPlantAt(thePlant->mPlantCol, thePlant->mRow, PlantPriority::TOPPLANT_ZEN_TOOL_ORDER) == thePlant)
    {
        return aRect.Contains(thePlant->mX + 40, thePlant->mY + 40);
    }
    return false;
    */

    const int aMinX = theMouseX - 70;
    const int aMaxX = theMouseX + 90;
    const int aMinY = theMouseY - 80;
    const int aMaxY = theMouseY + 80;

    if (GetTopPlantAt(thePlant->mPlantCol, thePlant->mRow, PlantPriority::TOPPLANT_ZEN_TOOL_ORDER) == thePlant) {
        return thePlant->mX + 40 >= aMinX && thePlant->mX + 40 < aMaxX && thePlant->mY + 40 >= aMinY &&
               thePlant->mY + 40 < aMaxY;
    }
    return false;
}

// 0x40E940
void Board::HighlightPlantsForMouse(const int theMouseX, const int theMouseY) {
    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WATERING_CAN &&
        mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_GOLD_WATERINGCAN)]) {
        Plant *aPlant = nullptr;
        while (IteratePlants(aPlant)) {
            if (IsPlantInGoldWateringCanRange(theMouseX, theMouseY, aPlant)) {
                aPlant->mHighlighted = true;
                Plant *aFlowerPot =
                    GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_UNDER_PLANT);
                if (aFlowerPot) {
                    aFlowerPot->mHighlighted = true;
                }
            }
        }
    } else {
        Plant *aPlant = ToolHitTest(theMouseX, theMouseY);
        if (aPlant) {
            aPlant->mHighlighted = true;
            if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
                Plant *aFlowerPot =
                    GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_UNDER_PLANT);
                if (aFlowerPot) {
                    aFlowerPot->mHighlighted = true;
                }
            }
        }
    }
}

// 0x40EAB0
void Board::UpdateMousePosition() {
    UpdateCursor();
    UpdateToolTip();
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        aPlant->mHighlighted = false;
    }

    const SeedType aCursorSeedType = GetSeedTypeInCursor();
    const int aMouseX = mApp->mWidgetManager->mLastMouseX - mX;
    const int aMouseY = mApp->mWidgetManager->mLastMouseY - mY;

    // 破罐者关卡中，检测并高亮鼠标悬浮的罐子
    if (mApp->IsScaryPotterLevel()) {
        GridItem *aGridItem = nullptr;
        while (IterateGridItems(aGridItem)) {
            if (aGridItem->mGridItemType == GridItemType::GRIDITEM_SCARY_POT) {
                aGridItem->mHighlighted = false;
            }
        }

        HitResult aHitResult;
        MouseHitTest(aMouseX, aMouseY, &aHitResult);
        if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_SCARY_POT) {
            const auto aScaryPot = static_cast<GridItem *>(aHitResult.mObject);
            aScaryPot->mHighlighted = true;
            return;
        }
    }

    // 禅境花园，设定蜗牛的高亮与否
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        GridItem *aStinky = mApp->mZenGarden->GetStinky();
        if (aStinky) {
            HitResult aHitResult;
            MouseHitTest(aMouseX, aMouseY, &aHitResult);
            aStinky->mHighlighted = aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_STINKY;
        }
    }

    // 手持铲子或花园工具时，令作用的植物高亮
    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_SHOVEL ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WATERING_CAN ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_FERTILIZER ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_BUG_SPRAY ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PHONOGRAPH ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_CHOCOLATE ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_GLOVE ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_MONEY_SIGN ||
        (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WHEEELBARROW &&
         !mApp->mZenGarden->GetPottedPlantInWheelbarrow())) {
        HighlightPlantsForMouse(aMouseX, aMouseY);
        return;
    }

    // 咖啡豆及坚果包扎术
    if (aCursorSeedType == SeedType::SEED_INSTANT_COFFEE) {
        const int aGridX =
            PlantingPixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);
        const int aGridY =
            PlantingPixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);

        Plant *aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
        if (aPlant && aPlant->mIsAsleep &&
            CanPlantAt(aGridX, aGridY, SeedType::SEED_INSTANT_COFFEE) == PlantingReason::PLANTING_OK) {
            aPlant->mHighlighted = true;
        }
    } else if (aCursorSeedType == SeedType::SEED_WALLNUT || aCursorSeedType == SeedType::SEED_TALLNUT) {
        const int aGridX =
            PlantingPixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);
        const int aGridY =
            PlantingPixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);

        Plant *aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_PUMPKIN);
        if (aPlant && aPlant->mSeedType == aCursorSeedType &&
            CanPlantAt(aGridX, aGridY, aCursorSeedType) == PlantingReason::PLANTING_OK) {
            aPlant->mHighlighted = true;
        }
    } else if (aCursorSeedType == SeedType::SEED_PUMPKINSHELL) {
        const int aGridX =
            PlantingPixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);
        const int aGridY =
            PlantingPixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);

        Plant *aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
        if (aPlant && aPlant->mSeedType == SeedType::SEED_PUMPKINSHELL &&
            CanPlantAt(aGridX, aGridY, SeedType::SEED_PUMPKINSHELL) == PlantingReason::PLANTING_OK) {
            aPlant->mHighlighted = true;
        }
    }
}

// 0x40EF00
void Board::UpdateToolTip() {
    if (!mApp->mWidgetManager->mMouseIn || !mApp->mActive || mBoardData.mTimeStopCounter > 0 ||
        mApp->GetDialogCount() > 0 || mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) {
        mToolTip->mVisible = false;
        return;
    }

    if (mWidgetManager == nullptr) return;

    const int aMouseX = mWidgetManager->mLastMouseX - mX;
    const int aMouseY = mWidgetManager->mLastMouseY - mY;

    if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO) {
        if (!mCutScene->mSeedChoosing) {
            mToolTip->mVisible = false;
            return;
        }

        if (mSeedBank->ContainsPoint(mWidgetManager->mLastMouseX, mWidgetManager->mLastMouseY) ||
            mApp->mSeedChooserScreen->mAlmanacButton->IsMouseOver() ||
            mApp->mSeedChooserScreen->mStoreButton->IsMouseOver() ||
            mApp->mSeedChooserScreen->mImitaterButton->IsMouseOver()) {
            mToolTip->mVisible = false;
            return;
        }

        Zombie *aZombie = ZombieHitTest(aMouseX, aMouseY);
        if (aZombie == nullptr || aZombie->mFromWave != Zombie::ZOMBIE_WAVE_CUTSCENE) {
            mToolTip->mVisible = false;
            return;
        }

        const SexyString aZombieName = fmt::format(_S("[{}]"), GetZombieDefinition(aZombie->mZombieType).mZombieName);
        mToolTip->SetTitle(aZombieName);
        if (mApp->CanShowAlmanac() && aZombie->mZombieType != ZombieType::ZOMBIE_GARGANTUAR) {
            mToolTip->SetLabel(_S("[CLICK_TO_VIEW]"));
        } else {
            mToolTip->SetLabel(_S(""));
        }
        mToolTip->SetWarningText(_S(""));

        const Rect aRect = aZombie->GetZombieRect();
        mToolTip->mX = aRect.mWidth / 2 + aRect.mX + 5;
        mToolTip->mY = aRect.mHeight + aRect.mY - 10;
        if (aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE) {
            mToolTip->mY = aZombie->mY;
        }

        mToolTip->mVisible = true;
        mToolTip->mCenter = true;

        mToolTip->mMinLeft = IMAGE_SEEDCHOOSER_BACKGROUND->GetWidth();
        if (mApp->mSeedChooserScreen->mAlmanacButton->mBtnNoDraw &&
            mApp->mSeedChooserScreen->mStoreButton->mBtnNoDraw) {
            mToolTip->mMaxBottom = 600;
        } else {
            mToolTip->mMaxBottom = 570;
        }
        if (!mApp->mSeedChooserScreen->mImitaterButton->mBtnNoDraw) {
            mToolTip->CalculateSize();
            if (mX + mToolTip->mX - mToolTip->mWidth / 2 < 524) {
                mToolTip->mMaxBottom = 503;
            }
        }

        return;
    }

    if (!CanInteractWithBoardButtons()) {
        mToolTip->mVisible = false;
        return;
    }

    mToolTip->mMinLeft = 0;
    mToolTip->mMaxBottom = BOARD_HEIGHT;
    mToolTip->SetTitle(_S(""));
    mToolTip->SetLabel(_S(""));
    mToolTip->SetWarningText(_S(""));
    mToolTip->mCenter = false;
    if (mChallenge->UpdateToolTip(aMouseX, aMouseY)) {
        return;
    }

    HitResult aHitResult;
    MouseHitTest(aMouseX, aMouseY, &aHitResult);

    if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_SHOVEL) {
        mToolTip->SetLabel(_S("[SHOVEL_TOOLTIP]"));
        const Rect aShovelButtonRect = GetShovelButtonRect();
        mToolTip->mX = aShovelButtonRect.mX + 35;
        mToolTip->mY = aShovelButtonRect.mY + 72;
        mToolTip->mCenter = true;
        mToolTip->mVisible = true;
        return;
    }

    if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_NEXT_GARDEN) {
        mToolTip->SetLabel(_S("[NEXT_GARDEN_TOOLTIP]"));
        const Rect aButtonRect = GetShovelButtonRect();
        mToolTip->mX = 599;
        mToolTip->mY = aButtonRect.mY + 52;
        mToolTip->mCenter = true;
        mToolTip->mVisible = true;
        return;
    }

    if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_WATERING_CAN ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_FERTILIZER ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_BUG_SPRAY ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_PHONOGRAPH ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_CHOCOLATE ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_GLOVE ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_MONEY_SIGN ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_WHEELBARROW ||
        aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_TREE_FOOD) {
        if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_WATERING_CAN) {
            mToolTip->SetLabel(_S("[WATERING_CAN_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_FERTILIZER) {
            mToolTip->SetLabel(_S("[FERTILIZER_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_BUG_SPRAY) {
            mToolTip->SetLabel(_S("[BUG_SPRAY_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_PHONOGRAPH) {
            mToolTip->SetLabel(_S("[PHONOGRAPH_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_CHOCOLATE) {
            mToolTip->SetLabel(_S("[CHOCOLATE_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_GLOVE) {
            mToolTip->SetLabel(_S("[GLOVE_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_MONEY_SIGN) {
            mToolTip->SetLabel(_S("[MONEY_SIGN_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_WHEELBARROW) {
            mToolTip->SetLabel(_S("[WHEELBARROW_TOOLTIP]"));
        } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_TREE_FOOD) {
            mToolTip->SetLabel(_S("[TREE_FERTILIZER_TOOLTIP]"));
        }

        Rect aButtonRect = GetShovelButtonRect();
        GetZenButtonRect(aHitResult.mObjectType, aButtonRect);
        this->mToolTip->mX = aButtonRect.mX + 35;
        this->mToolTip->mY = aButtonRect.mY + 72;
        this->mToolTip->mCenter = true;
        this->mToolTip->mVisible = true;
        return;
    }

    if (aHitResult.mObjectType != GameObjectType::OBJECT_TYPE_SEEDPACKET) {
        mToolTip->mVisible = false;
        return;
    }

    const auto aSeedPacket = static_cast<SeedPacket *>(aHitResult.mObject);
    SeedType aUseSeedType = aSeedPacket->mPacketType;
    if (aSeedPacket->mPacketType == SeedType::SEED_IMITATER && aSeedPacket->mImitaterType != SeedType::SEED_NONE) {
        aUseSeedType = aSeedPacket->mImitaterType;
    }

    if (gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST) {
        if (aUseSeedType == SeedType::SEED_REPEATER) {
            mToolTip->SetLabel(_S("[BEGHOULED_REPEATER_UPGRADE_TOOLTIP]"));
        } else if (aUseSeedType == SeedType::SEED_FUMESHROOM) {
            mToolTip->SetLabel(_S("[BEGHOULED_FUMESHROOM_UPGRADE_TOOLTIP]"));
        } else if (aUseSeedType == SeedType::SEED_TALLNUT) {
            mToolTip->SetLabel(_S("[BEGHOULED_TALLNUT_UPGRADE_TOOLTIP]"));
        } else if (aUseSeedType == SeedType::SEED_BEGHOULED_BUTTON_SHUFFLE) {
            mToolTip->SetLabel(_S("[BEGHOULED_SHUFFLE_TOOLTIP]"));
        } else if (aUseSeedType == SeedType::SEED_BEGHOULED_BUTTON_CRATER) {
            mToolTip->SetLabel(_S("[BEGHOULED_CRATER_TOOLTIP]"));
        }
    } else if (aUseSeedType == SeedType::SEED_SLOT_MACHINE_SUN) {
        mToolTip->SetLabel(_S("[SLOT_MACHINE_SUN_TOOLTIP]"));
    } else if (aUseSeedType == SeedType::SEED_SLOT_MACHINE_DIAMOND) {
        mToolTip->SetLabel(_S("[SLOT_MACHINE_DIAMOND_TOOLTIP]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIQUARIUM_SNORKLE) {
        mToolTip->SetLabel(_S("[ZOMBIQUARIUM_SNORKEL_TOOLTIP]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIQUARIUM_TROPHY) {
        mToolTip->SetLabel(_S("[ZOMBIQUARIUM_TROPHY_TOOLTIP]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_NORMAL) {
        mToolTip->SetLabel(_S("[ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_TRAFFIC_CONE) {
        mToolTip->SetLabel(_S("[CONEHEAD_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_POLEVAULTER) {
        mToolTip->SetLabel(_S("[POLE_VAULTING_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_PAIL) {
        mToolTip->SetLabel(_S("[BUCKETHEAD_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_LADDER) {
        mToolTip->SetLabel(_S("[LADDER_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_DIGGER) {
        mToolTip->SetLabel(_S("[DIGGER_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_BUNGEE) {
        mToolTip->SetLabel(_S("[BUNGEE_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_FOOTBALL) {
        mToolTip->SetLabel(_S("[FOOTBALL_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_BALLOON) {
        mToolTip->SetLabel(_S("[BALLOON_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_SCREEN_DOOR) {
        mToolTip->SetLabel(_S("[SCREEN_DOOR_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBONI) {
        mToolTip->SetLabel(_S("[ZOMBONI]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_POGO) {
        mToolTip->SetLabel(_S("[POGO_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_DANCER) {
        mToolTip->SetLabel(_S("[DANCING_ZOMBIE]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_GARGANTUAR) {
        mToolTip->SetLabel(_S("[GARGANTUAR]"));
    } else if (aUseSeedType == SeedType::SEED_ZOMBIE_IMP) {
        mToolTip->SetLabel(_S("[IMP]"));
    } else {
        // @Patoke: wrong function call
        mToolTip->SetLabel(Plant::GetNameString(aUseSeedType));
    }

    const int aPlantCost = GetCurrentPlantCost(aSeedPacket->mPacketType, aSeedPacket->mImitaterType);
    if (mApp->mEasyPlantingCheat) {
        mToolTip->SetWarningText(_S("FREE_PLANTING_CHEAT"));
    } else if (!aSeedPacket->mActive && (gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)) {
        if (aSeedPacket->mPacketType == SeedType::SEED_BEGHOULED_BUTTON_CRATER) {
            mToolTip->SetWarningText(_S("[BEGHOULED_NO_CRATERS]"));
        } else {
            mToolTip->SetWarningText(_S("[BEGHOULED_SEED_ALREADY_PURCHASED]"));
        }
    } else if (!aSeedPacket->mActive) {
        mToolTip->SetWarningText(_S("[WAITING_FOR_SEED]"));
    } else if (!CanTakeSunMoney(aPlantCost) && !HasConveyorBeltSeedBank() && !mApp->IsSlotMachineLevel()) {
        mToolTip->SetWarningText(_S("[NOT_ENOUGH_SUN]"));
    } else if (aUseSeedType == SeedType::SEED_GATLINGPEA) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_REPEATER]"));
        }
    } else if (aUseSeedType == SeedType::SEED_WINTERMELON) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_MELONPULT]"));
        }
    } else if (aUseSeedType == SeedType::SEED_TWINSUNFLOWER) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_SUNFLOWER]"));
        }
    } else if (aUseSeedType == SeedType::SEED_SPIKEROCK) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_SPIKEWEED]"));
        }
    } else if (aUseSeedType == SeedType::SEED_COBCANNON) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_KERNELPULTS]"));
        }
    } else if (aUseSeedType == SeedType::SEED_GOLD_MAGNET) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_MAGNETSHROOM]"));
        }
    } else if (aUseSeedType == SeedType::SEED_GLOOMSHROOM) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_FUMESHROOM]"));
        }
    } else if (aUseSeedType == SeedType::SEED_CATTAIL) {
        if (!PlantingRequirementsMet(aUseSeedType)) {
            mToolTip->SetWarningText(_S("[REQUIRES_LILY_PAD]"));
        }
    }

    mToolTip->mX = (SEED_PACKET_WIDTH - mToolTip->mWidth) / 2 + mSeedBank->mX + aSeedPacket->mOffsetX + aSeedPacket->mX;
    mToolTip->mY = mSeedBank->mY + aSeedPacket->mY + 70;
    mToolTip->mVisible = true;
}

// 0x40FC70
void Board::MouseDownCobcannonFire(const int x, const int y, const int theClickCount) {
    if (theClickCount >= 0 && y >= 80) {
        if (mBoardData.mCobCannonCursorDelayCounter > 0 &&
            Distance2D(x, y, mBoardData.mCobCannonMouseX, mBoardData.mCobCannonMouseY) < 100.0f) {
            return; // 误点检测：点击加农炮后的 30cs 内，点击的位置和准心位置之间的距离小于 100 时，将被判定为误点
        }

        if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_DUPLICATOR) {
            Plant *aCobcannon = mPlants.DataArrayTryToGet(mCursorObject->mCobCannonPlantID);
            if (aCobcannon) {
                aCobcannon->CobCannonFire(x, y);
            }
        }
    }
    ClearCursor();
}

// 0x40FD30
//  GOTY @Patoke: 0x4126F0
void Board::MouseDownWithPlant(const int x, const int y, const int theClickCount) {
    // 右击鼠标：放下卡牌
    if (theClickCount < 0) {
        RefreshSeedPacketFromCursor();
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        return;
    }

    // 我是僵尸模式中，交由 Challenge 处理
    if (mApp->IsIZombieLevel()) {
        mChallenge->IZombieMouseDownWithZombie(x, y, theClickCount);
        return;
    }

    const SeedType aPlantingSeedType = GetSeedTypeInCursor();
    const int aGridX = PlantingPixelToGridX(x, y, aPlantingSeedType);
    const int aGridY = PlantingPixelToGridY(x, y, aPlantingSeedType);

    // 不在场地内的点击：放下卡牌
    if (aGridX < 0 || aGridX >= MAX_GRID_SIZE_X || aGridY < 0 || aGridY > MAX_GRID_SIZE_Y) {
        RefreshSeedPacketFromCursor();
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        return;
    }

    const PlantingReason aReason = CanPlantAt(aGridX, aGridY, aPlantingSeedType);
    if (aReason != PlantingReason::PLANTING_OK) {
        // 根据不同的种植原因播放相应的提示字幕
        if (aReason == PlantingReason::PLANTING_ONLY_ON_GRAVES) {
            DisplayAdvice(
                _S("[ADVICE_GRAVEBUSTERS_ON_GRAVES]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                AdviceType::ADVICE_PLANT_GRAVEBUSTERS_ON_GRAVES
            );
        } else if (aPlantingSeedType == SeedType::SEED_LILYPAD) {
            if (aReason == PlantingReason::PLANTING_ONLY_IN_POOL) {
                DisplayAdvice(
                    _S("[ADVICE_LILYPAD_ON_WATER]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_LILYPAD_ON_WATER
                );
            }
        } else if (aPlantingSeedType == SeedType::SEED_TANGLEKELP) {
            if (aReason == PlantingReason::PLANTING_ONLY_IN_POOL) {
                DisplayAdvice(
                    _S("[ADVICE_TANGLEKELP_ON_WATER]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_TANGLEKELP_ON_WATER
                );
            }
        } else if (aPlantingSeedType == SeedType::SEED_SEASHROOM) {
            if (aReason == PlantingReason::PLANTING_ONLY_IN_POOL) {
                DisplayAdvice(
                    _S("[ADVICE_SEASHROOM_ON_WATER]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_SEASHROOM_ON_WATER
                );
            }
        } else if (aReason == PlantingReason::PLANTING_ONLY_ON_GROUND) {
            DisplayAdvice(
                _S("[ADVICE_POTATO_MINE_ON_LILY]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY
            );
        } else if (aReason == PlantingReason::PLANTING_NOT_PASSED_LINE) {
            DisplayAdvice(
                _S("[ADVICE_NOT_PASSED_LINE]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                AdviceType::ADVICE_PLANT_NOT_PASSED_LINE
            );
        } else if (aReason == PlantingReason::PLANTING_NEEDS_UPGRADE) {
            switch (aPlantingSeedType) {
            case SeedType::SEED_GATLINGPEA:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_REPEATERS]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_REPEATERS
                );
                break;

            case SeedType::SEED_TWINSUNFLOWER:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_SUNFLOWER]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_SUNFLOWER
                );
                break;

            case SeedType::SEED_GLOOMSHROOM:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_FUMESHROOM]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_FUMESHROOM
                );
                break;

            case SeedType::SEED_CATTAIL:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_LILYPAD]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_LILYPAD
                );
                break;

            case SeedType::SEED_WINTERMELON:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_MELONPULT]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_MELONPULT
                );
                break;

            case SeedType::SEED_GOLD_MAGNET:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_MAGNETSHROOM]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_MAGNETSHROOM
                );
                break;

            case SeedType::SEED_SPIKEROCK:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_SPIKEWEED]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_SPIKEWEED
                );
                break;

            case SeedType::SEED_COBCANNON:
                DisplayAdvice(
                    _S("[ADVICE_ONLY_ON_KERNELPULT]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_ONLY_ON_KERNELPULT
                );
                break;
            default: break;
            }
        } else if (aReason == PlantingReason::PLANTING_NOT_ON_ART) {
            const SexyString aSeedName =
                Plant::GetNameString(mChallenge->GetArtChallengeSeed(aGridX, aGridY), SeedType::SEED_NONE);
            const SexyString aMessage = TodReplaceString(_S("[ADVICE_WRONG_ART_TYPE]"), _S("{SEED}"), aSeedName);
            DisplayAdvice(aMessage, MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_WRONG_ART_TYPE);
        } else if (aReason == PlantingReason::PLANTING_NEEDS_POT) {
            if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 41) {
                DisplayAdvice(
                    _S("[ADVICE_PLANT_NEED_POT1]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_NEED_POT
                );
            } else {
                DisplayAdvice(
                    _S("[ADVICE_PLANT_NEED_POT2]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_NEED_POT
                );
            }
        } else if (aReason == PlantingReason::PLANTING_NOT_ON_GRAVE) {
            DisplayAdvice(
                _S("[ADVICE_PLANT_NOT_ON_GRAVE]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                AdviceType::ADVICE_PLANT_NOT_ON_GRAVE
            );
        } else if (aReason == PlantingReason::PLANTING_NOT_ON_CRATER) {
            if (IsPoolSquare(aGridX, aGridY)) {
                DisplayAdvice(
                    _S("[ADVICE_CANT_PLANT_THERE]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_CANT_PLANT_THERE
                );
            } else {
                DisplayAdvice(
                    _S("[ADVICE_PLANT_NOT_ON_CRATER]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_NOT_ON_CRATER
                );
            }
        } else if (aReason == PlantingReason::PLANTING_NOT_ON_WATER) {
            if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
                mApp->mZenGarden->mGardenType == GardenType::GARDEN_AQUARIUM) {
                DisplayAdvice(
                    _S("[ZEN_ONLY_AQUATIC_PLANTS]"), MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_NONE
                );
            } else if (aPlantingSeedType == SeedType::SEED_POTATOMINE) {
                DisplayAdvice(
                    _S("[ADVICE_POTATO_MINE_ON_LILY]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY
                );
            } else {
                DisplayAdvice(
                    _S("[ADVICE_PLANT_NOT_ON_WATER]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                    AdviceType::ADVICE_PLANT_NOT_ON_WATER
                );
            }
        } else if (aReason == PlantingReason::PLANTING_NEEDS_GROUND) {
            DisplayAdvice(
                _S("[ADVICE_PLANTING_NEEDS_GROUND]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                AdviceType::ADVICE_PLANTING_NEEDS_GROUND
            );
        } else if (aReason == PlantingReason::PLANTING_NEEDS_SLEEPING) {
            DisplayAdvice(
                _S("[ADVICE_PLANTING_NEED_SLEEPING]"), MessageStyle::MESSAGE_STYLE_HINT_FAST,
                AdviceType::ADVICE_PLANTING_NEED_SLEEPING
            );
        }

        // 特定情况下，放下原有手持的植物
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE || mApp->IsWhackAZombieLevel()) {
            RefreshSeedPacketFromCursor();
            mApp->PlayFoley(FoleyType::FOLEY_DROP);
        }
        // 不可种植的情况至此结束，直接跳转至返回
        return;
    }

    /* 以下为植物类型可以种植的情况 */
    // 清除种植相关的提示字幕
    ClearAdvice(AdviceType::ADVICE_PLANTING_NEED_SLEEPING);
    ClearAdvice(AdviceType::ADVICE_CANT_PLANT_THERE);
    ClearAdvice(AdviceType::ADVICE_PLANTING_NEEDS_GROUND);
    ClearAdvice(AdviceType::ADVICE_PLANT_NOT_ON_WATER);
    ClearAdvice(AdviceType::ADVICE_PLANT_NOT_ON_CRATER);
    ClearAdvice(AdviceType::ADVICE_PLANT_NOT_ON_GRAVE);
    ClearAdvice(AdviceType::ADVICE_PLANT_NEED_POT);
    ClearAdvice(AdviceType::ADVICE_PLANT_WRONG_ART_TYPE);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_LILYPAD);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_MAGNETSHROOM);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_FUMESHROOM);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_KERNELPULT);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_SUNFLOWER);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_SPIKEWEED);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_MELONPULT);
    ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_REPEATERS);
    ClearAdvice(AdviceType::ADVICE_PLANT_NOT_PASSED_LINE);
    ClearAdvice(AdviceType::ADVICE_PLANT_GRAVEBUSTERS_ON_GRAVES);
    ClearAdvice(AdviceType::ADVICE_PLANT_LILYPAD_ON_WATER);
    ClearAdvice(AdviceType::ADVICE_PLANT_TANGLEKELP_ON_WATER);
    ClearAdvice(AdviceType::ADVICE_PLANT_SEASHROOM_ON_WATER);
    ClearAdvice(AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY);
    ClearAdvice(AdviceType::ADVICE_SURVIVE_FLAGS);

    // 无免费种植、非传送带关卡的卡槽植物，判断阳光是否充足：充足则扣除阳光，不足则退出
    if (!mApp->mEasyPlantingCheat && mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK &&
        !HasConveyorBeltSeedBank()) {
        if (!TakeSunMoney(GetCurrentPlantCost(aPlantingSeedType, SeedType::SEED_NONE))) {
            return;
        }
    }

    // 升级种植或坚果包扎术等情况时，先将原植物销毁
    bool aIsAwake = false;
    int aWakeUpCounter = 0;
    PlantsOnLawn aPlantOnLawn;
    GetPlantsOnLawn(aGridX, aGridY, &aPlantOnLawn);
    Plant *aNormalPlant = aPlantOnLawn.mNormalPlant;
    Plant *aPumpkinPlant = aPlantOnLawn.mPumpkinPlant;
    if (aNormalPlant && aNormalPlant->IsUpgradableTo(aPlantingSeedType)) {
        if (aPlantingSeedType == SeedType::SEED_GLOOMSHROOM) {
            aIsAwake = !aNormalPlant->mIsAsleep;
            aWakeUpCounter = aNormalPlant->mWakeUpCounter;
        }
        aNormalPlant->Die();
    }
    if ((aPlantingSeedType == SeedType::SEED_WALLNUT || aPlantingSeedType == SeedType::SEED_TALLNUT) && aNormalPlant) {
        if (aNormalPlant->mSeedType == aPlantingSeedType) {
            aNormalPlant->Die();
        }
    }
    if (aPlantingSeedType == SeedType::SEED_PUMPKINSHELL && aPumpkinPlant) {
        if (aPumpkinPlant->mSeedType == SeedType::SEED_PUMPKINSHELL) {
            aPumpkinPlant->Die();
        }
    }
    if (aPlantingSeedType == SeedType::SEED_COBCANNON) {
        Plant *aRightPlant = GetTopPlantAt(aGridX + 1, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
        if (aRightPlant) {
            aRightPlant->Die();
        }
    }
    if (aPlantingSeedType == SeedType::SEED_CATTAIL) {
        if (aPlantOnLawn.mUnderPlant) {
            aPlantOnLawn.mUnderPlant->Die();
        }
        if (aNormalPlant) {
            aNormalPlant->Die();
        }
    }

    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE) {
        mApp->mZenGarden->MovePlant(mPlants.DataArrayTryToGet(mCursorObject->mGlovePlantID), aGridX, aGridY);
    } else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW) {
        mApp->mZenGarden->MouseDownWithFullWheelBarrow(x, y);
    } else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN) {
        AddPlant(aGridX, aGridY, mCursorObject->mType, mCursorObject->mImitaterType);
        Coin *aCoin = mCoins.DataArrayTryToGet(mCursorObject->mCoinID);
        mCursorObject->mCoinID = CoinID::COINID_NULL;
        aCoin->Die();
    } else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK) {
        Plant *aPlant = AddPlant(aGridX, aGridY, mCursorObject->mType, mCursorObject->mImitaterType);
        if (aIsAwake) {
            aPlant->SetSleeping(false);
        } else {
            aPlant->mWakeUpCounter = aWakeUpCounter;
        }

        mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].WasPlanted();
    } else {
        TOD_ASSERT();
    }

    // 柱子关卡中，一列种植
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN) {
        for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
            if (aRow == aGridY || CanPlantAt(aGridX, aRow, aPlantingSeedType) != PlantingReason::PLANTING_OK) continue;

            if (aPlantingSeedType == SeedType::SEED_WALLNUT || aPlantingSeedType == SeedType::SEED_TALLNUT) {
                aNormalPlant = GetTopPlantAt(aGridX, aRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
                if (aNormalPlant && aNormalPlant->mSeedType == aPlantingSeedType) {
                    aNormalPlant->Die();
                }
            }
            if (aPlantingSeedType == SeedType::SEED_PUMPKINSHELL) {
                aPumpkinPlant = GetTopPlantAt(aGridX, aRow, PlantPriority::TOPPLANT_ONLY_PUMPKIN);
                if (aPumpkinPlant && aPumpkinPlant->mSeedType == SeedType::SEED_PUMPKINSHELL) {
                    aPumpkinPlant->Die();
                }
            }
            AddPlant(aGridX, aRow, mCursorObject->mType, mCursorObject->mImitaterType);
        }
    }

    // 设置教程状态相关
    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER) {
        SetTutorialState(
            mPlants.mSize >= 2 ? TutorialState::TUTORIAL_LEVEL_1_COMPLETED
                               : TutorialState::TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER
        );
    } else if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER) {
        const int aSunFlowersCount = CountSunFlowers();
        if (aPlantingSeedType == SeedType::SEED_SUNFLOWER && aSunFlowersCount == 2) {
            DisplayAdvice(
                _S("[ADVICE_MORE_SUNFLOWERS]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE
            );
            if (!mSeedBank->mSeedPackets[1].CanPickUp()) {
                SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER);
            } else {
                SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
            }
        } else if (aSunFlowersCount >= 3) {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_COMPLETED);
        } else if (!mSeedBank->mSeedPackets[1].CanPickUp()) {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER);
        } else {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
        }
    } else if (mBoardData.mTutorialState == TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER) {
        if (CountSunFlowers() >= 3) {
            SetTutorialState(TutorialState::TUTORIAL_MORESUN_COMPLETED);
            DisplayAdvice(
                _S("[ADVICE_PLANT_SUNFLOWER5]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER,
                AdviceType::ADVICE_PLANT_SUNFLOWER5
            );
            mBoardData.mTutorialTimer = -1;
        } else if (!mSeedBank->mSeedPackets[1].CanPickUp()) {
            SetTutorialState(TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER);
        } else {
            SetTutorialState(TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER);
        }
    }

    // 保龄球关卡，播放保龄球滚动的音效
    if (mApp->IsWallnutBowlingLevel()) {
        mApp->PlaySample(Sexy::SOUND_BOWLING);
    }

    // 重置鼠标
    ClearCursor();
}

// 0x410F70
Plant *Board::ToolHitTestHelper(HitResult *theHitResult) {
    theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_PLANT;
    const auto aPlant = static_cast<Plant *>(theHitResult->mObject);
    return (aPlant->mSeedType != SeedType::SEED_GRAVEBUSTER ||
            mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
               ? aPlant
               : nullptr;
}

Plant *Board::ToolHitTest(const int theX, const int theY) {
    HitResult aHitResult;
    MouseHitTest(theX, theY, &aHitResult);
    if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_PLANT) {
        return ToolHitTestHelper(&aHitResult);
    }
    return nullptr;
}

// 0x410FA0
void Board::TutorialArrowShow(const int theX, const int theY) {
    TutorialArrowRemove();
    TodParticleSystem *aParticle = mApp->AddTodParticle(
        theX, theY, MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ParticleEffect::PARTICLE_SEED_PACKET_PICK
    );
    mBoardData.mTutorialParticleID = mApp->ParticleGetID(aParticle);
}

// 0x411020
void Board::TutorialArrowRemove() {
    mApp->RemoveParticle(mBoardData.mTutorialParticleID);
    mBoardData.mTutorialParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
}

// 0x411060
void Board::MouseDownWithTool(const int x, const int y, const int theClickCount, const CursorType theCursorType) {
    if (theClickCount < 0) {
        ClearCursor();
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        return;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        mApp->mZenGarden->MouseDownWithTool(x, y, theCursorType);
        return;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        mChallenge->TreeOfWisdomTool(x, y);
        return;
    }

    Plant *aPlant = ToolHitTest(x, y);
    if (aPlant == nullptr) {
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
    } else if (theCursorType == CursorType::CURSOR_TYPE_SHOVEL) {
        mApp->PlayFoley(FoleyType::FOLEY_USE_SHOVEL);
        mBoardData.mPlantsShoveled++;
        aPlant->Die();

        if (aPlant->mSeedType == SeedType::SEED_CATTAIL &&
            GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_PUMPKIN)) {
            NewPlant(aPlant->mPlantCol, aPlant->mRow, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
        }
        if (mBoardData.mTutorialState == TutorialState::TUTORIAL_SHOVEL_DIG ||
            mBoardData.mTutorialState == TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING) {
            SetTutorialState(
                CountPlantByType(SeedType::SEED_PEASHOOTER) == 0 ? TutorialState::TUTORIAL_SHOVEL_COMPLETED
                                                                 : TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING
            );
        }
    }

    ClearCursor();
}

// 0x411280
Plant *Board::SpecialPlantHitTest(const int x, const int y) {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mSeedType == SeedType::SEED_PUMPKINSHELL) {
            const float aMinDist =
                GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION) ? 25 : 0;
            const float aDistance = Distance2D(x, y, aPlant->mX + 40, aPlant->mY + 40);
            if (aDistance >= aMinDist && aDistance <= 50 && y > aPlant->mY + 25) {
                return aPlant;
            }
        } else if (Plant::IsFlying(aPlant->mSeedType)) {
            if (Distance2D(x, y, aPlant->mX + 40, aPlant->mY) < 15) {
                return aPlant;
            }
        }
    }
    return nullptr;
}

// 0x411470
bool Board::MouseHitTestPlant(const int x, const int y, HitResult *theHitResult) {
    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_COBCANNON_TARGET ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
        return false;

    Plant *aPlant = SpecialPlantHitTest(x, y);
    if (aPlant) {
        theHitResult->mObject = aPlant;
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_PLANT;
        return true;
    }

    const int aGridX = PixelToGridX(x, y);
    const int aGridY = PixelToGridY(x, y);
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ZEN_TOOL_ORDER);
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WATERING_CAN &&
            (!aPlant || !mApp->mZenGarden->PlantCanBeWatered(aPlant))) {
            Plant *aTopPlant = GetTopPlantAt(
                PixelToGridX(x - 30, y - 20), PixelToGridY(x - 30, y - 20), PlantPriority::TOPPLANT_ZEN_TOOL_ORDER
            );
            if (aTopPlant && mApp->mZenGarden->PlantCanBeWatered(aTopPlant)) {
                aPlant = aTopPlant;
            }
        }
    } else {
        aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_DIGGING_ORDER);
        if (aPlant && (aPlant->mSeedType == SeedType::SEED_LILYPAD || aPlant->mSeedType == SeedType::SEED_FLOWERPOT)) {
            if (GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_PUMPKIN)) {
                return false;
            }
        }
    }

    // 植物不存在，或者手持巧克力但植物不需要巧克力时，返回“否”
    if (aPlant == nullptr) {
        return false;
    }
    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_CHOCOLATE &&
        !mApp->mZenGarden->PlantCanHaveChocolate(aPlant)) {
        theHitResult->mObject = nullptr;
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
        return false;
    }

    theHitResult->mObject = aPlant;
    theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_PLANT;
    return true;
}

// 0x411730
bool Board::MouseHitTest(const int x, const int y, HitResult *theHitResult) {
    if (mBoardData.mBoardFadeOutCounter >= 0 || IsScaryPotterDaveTalking()) {
        theHitResult->mObject = nullptr;
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
        return false;
    }

    if (mMenuButton->IsMouseOver() && CanInteractWithBoardButtons()) {
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_MENU_BUTTON;
        return true;
    }

    if (mStoreButton && mStoreButton->IsMouseOver() && CanInteractWithBoardButtons()) {
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_STORE_BUTTON;
        return true;
    }

    const Rect aShovelButtonRect = GetShovelButtonRect();
    if (mSeedBank->MouseHitTest(x, y, theHitResult)) {
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL ||
            mCursorObject->mCursorType == CursorType::CURSOR_TYPE_COBCANNON_TARGET ||
            mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
            return true;
    }
    if (mBoardData.mShowShovel && aShovelButtonRect.Contains(x, y) && CanInteractWithBoardButtons()) {
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_SHOVEL;
        return true;
    }

    if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL ||
        mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER) {
        Coin *aCoin = nullptr;
        const Coin *aTopCoin = nullptr;
        while (IterateCoins(aCoin)) {
            HitResult aHitResultCoin;
            if (aCoin->MouseHitTest(x, y, &aHitResultCoin)) {
                aCoin = static_cast<Coin *>(aHitResultCoin.mObject);
                if (aTopCoin == nullptr || aCoin->mRenderOrder >= aTopCoin->mRenderOrder) {
                    theHitResult->mObjectType = aHitResultCoin.mObjectType;
                    theHitResult->mObject = aCoin;
                    aTopCoin = aCoin;
                }
            }
        }
        if (aTopCoin) {
            return true;
        }
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        bool canClick = false;
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_CHOCOLATE &&
            !mApp->mZenGarden->IsStinkyHighOnChocolate()) {
            canClick = true;
        } else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL && mApp->mZenGarden->IsStinkySleeping()) {
            canClick = true;
        }

        const GridItem *aStinky = mApp->mZenGarden->GetStinky();
        if (canClick && aStinky) {
            const Rect aStinkyRect(aStinky->mPosX - 6, aStinky->mPosY - 10, 84, 90);
            if (aStinkyRect.Contains(x, y)) {
                theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_STINKY;
                return true;
            }
        }
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_TREE_FOOD &&
            mChallenge->TreeOfWisdomHitTest(x, y, theHitResult)) {
            return true;
        }
    }
    if ((mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
         mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) &&
        CanInteractWithBoardButtons()) {
        for (int i = (int)GameObjectType::OBJECT_TYPE_WATERING_CAN;
             i <= static_cast<int>(GameObjectType::OBJECT_TYPE_NEXT_GARDEN); i++) {
            const auto aTool = static_cast<GameObjectType>(i);
            if (CanUseGameObject(aTool) &&
                (aTool != GameObjectType::OBJECT_TYPE_TREE_FOOD || mChallenge->TreeOfWisdomCanFeed())) {
                Rect aButtonRect = GetShovelButtonRect();
                if (aTool == GameObjectType::OBJECT_TYPE_NEXT_GARDEN) {
                    aButtonRect.mX = 564;
                } else {
                    GetZenButtonRect(aTool, aButtonRect);
                }

                if (aButtonRect.Contains(x, y)) {
                    theHitResult->mObjectType = static_cast<GameObjectType>(aTool);
                    return true;
                }
            }
        }
    }

    if (MouseHitTestPlant(x, y, theHitResult)) return true;

    if (mApp->IsScaryPotterLevel() && mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL &&
        mChallenge->mChallengeState != ChallengeState::STATECHALLENGE_SCARY_POTTER_MALLETING &&
        mApp->mGameScene == GameScenes::SCENE_PLAYING && mApp->GetDialog(Dialogs::DIALOG_GAME_OVER) == nullptr &&
        mApp->GetDialog(Dialogs::DIALOG_CONTINUE) == nullptr) {
        GridItem *aScaryPot = GetGridItemAt(GridItemType::GRIDITEM_SCARY_POT, PixelToGridX(x, y), PixelToGridY(x, y));
        if (aScaryPot) {
            theHitResult->mObject = aScaryPot;
            theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_SCARY_POT;
            return true;
        }
    }

    if (mApp->IsSlotMachineLevel()) {
        const Rect aSlotMachineHandleRect = mChallenge->SlotMachineGetHandleRect();
        if (aSlotMachineHandleRect.Contains(x, y) &&
            mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_NORMAL && !HasLevelAwardDropped()) {
            theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_SLOT_MACHINE_HANDLE;
            return true;
        }
    }

    theHitResult->mObject = nullptr;
    theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
    return false;
}

// 0x411C10
void Board::PickUpTool(const GameObjectType theObjectType) {
    if (mBoardData.mPaused || (mApp->mGameScene != GameScenes::SCENE_PLAYING && !mCutScene->IsInShovelTutorial()))
        return;

    switch (theObjectType) {
    case GameObjectType::OBJECT_TYPE_SHOVEL:
        if (mBoardData.mTutorialState == TutorialState::TUTORIAL_SHOVEL_PICKUP) {
            SetTutorialState(TutorialState::TUTORIAL_SHOVEL_DIG);
        }
        mCursorObject->mCursorType = CursorType::CURSOR_TYPE_SHOVEL;
        mApp->PlayFoley(FoleyType::FOLEY_SHOVEL);
        break;

    case GameObjectType::OBJECT_TYPE_WATERING_CAN:
        if (mBoardData.mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_PICKUP_WATER) {
            mBoardData.mTutorialState = TutorialState::TUTORIAL_ZEN_GARDEN_WATER_PLANT;
            DisplayAdvice(
                _S("[ADVICE_ZEN_GARDEN_WATER_PLANT]"), MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG,
                AdviceType::ADVICE_NONE
            );
            TutorialArrowRemove();
        }
        mCursorObject->mCursorType = CursorType::CURSOR_TYPE_WATERING_CAN;
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        break;

    case GameObjectType::OBJECT_TYPE_FERTILIZER:
        if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_FERTILIZER) > 0) {
            mCursorObject->mCursorType = CursorType::CURSOR_TYPE_FERTILIZER;
        } else {
            mApp->PlaySample(Sexy::SOUND_BUZZER);
        }
        break;

    case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
        if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_BUG_SPRAY) > 0) {
            mCursorObject->mCursorType = CursorType::CURSOR_TYPE_BUG_SPRAY;
        } else {
            mApp->PlaySample(Sexy::SOUND_BUZZER);
        }
        break;

    case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
        mCursorObject->mCursorType = CursorType::CURSOR_TYPE_PHONOGRAPH;
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        break;

    case GameObjectType::OBJECT_TYPE_CHOCOLATE:
        if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_CHOCOLATE) > 0) {
            mCursorObject->mCursorType = CursorType::CURSOR_TYPE_CHOCOLATE;
        } else {
            mApp->PlaySample(Sexy::SOUND_BUZZER);
        }
        break;

    case GameObjectType::OBJECT_TYPE_GLOVE:
        mCursorObject->mCursorType = CursorType::CURSOR_TYPE_GLOVE;
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        break;

    case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
        mCursorObject->mCursorType = CursorType::CURSOR_TYPE_MONEY_SIGN;
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        break;

    case GameObjectType::OBJECT_TYPE_WHEELBARROW:
        mCursorObject->mCursorType = CursorType::CURSOR_TYPE_WHEEELBARROW;
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        break;

    case GameObjectType::OBJECT_TYPE_TREE_FOOD:
        if (mChallenge->TreeOfWisdomCanFeed()) {
            if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_TREE_FOOD) > 0) {
                mCursorObject->mCursorType = CursorType::CURSOR_TYPE_TREE_FOOD;
            } else {
                mApp->PlaySample(Sexy::SOUND_BUZZER);
            }
        }
        break;

    default: TOD_ASSERT(); break;
    }

    mCursorObject->mType = SeedType::SEED_NONE;
}

// 0x411F20
void Board::MouseDown(const int x, const int y, const int theClickCount) {
    Widget::MouseDown(x, y, theClickCount);
    mIgnoreMouseUp = !CanInteractWithBoardButtons();
    if (mBoardData.mTimeStopCounter > 0) return;

    HitResult aHitResult;
    MouseHitTest(x, y, &aHitResult);
    if (mChallenge->MouseDown(x, y, theClickCount, &aHitResult)) return;

    if (mMenuButton->IsMouseOver() && CanInteractWithBoardButtons() && theClickCount > 0) {
        mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
    } else if (mStoreButton && mStoreButton->IsMouseOver() && CanInteractWithBoardButtons() && theClickCount > 0) {
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
            mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
            mApp->PlaySample(Sexy::SOUND_TAP);
        } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mApp->mGameMode == GameMode::GAMEMODE_UPSELL) {
            mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
        }
    }

    if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && mApp->mSeedChooserScreen) {
        mApp->mSeedChooserScreen->CancelLawnView();
    }
    if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) {
        mCutScene->ZombieWonClick();
        return;
    }
    if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO) {
        mCutScene->MouseDown(x, y);
    }

    if (mApp->mTodCheatKeys && !mApp->IsScaryPotterLevel() && mBoardData.mNextSurvivalStageCounter > 0) {
        mBoardData.mNextSurvivalStageCounter = 2;
        for (int i = 0; i < MAX_GRID_SIZE_Y; i++) {
            if (mBoardData.mIceTimer[i] > 2) {
                mBoardData.mIceTimer[i] = 2;
            }
        }
    }

    const CursorType aCursor = mCursorObject->mCursorType;
    if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_NONE) {
        if (aCursor == CursorType::CURSOR_TYPE_COBCANNON_TARGET) {
            MouseDownCobcannonFire(x, y, theClickCount);
            UpdateCursor();
            return;
        }
    } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_COIN && theClickCount >= 0) {
        const auto aCoin = static_cast<Coin *>(aHitResult.mObject);
        if (aCoin->mBoard) {
            aCoin->MouseDown(x, y, theClickCount);
        }
        UpdateCursor();
        return;
    }

    if (aCursor == CursorType::CURSOR_TYPE_SHOVEL || aCursor == CursorType::CURSOR_TYPE_WATERING_CAN ||
        aCursor == CursorType::CURSOR_TYPE_FERTILIZER || aCursor == CursorType::CURSOR_TYPE_BUG_SPRAY ||
        aCursor == CursorType::CURSOR_TYPE_PHONOGRAPH || aCursor == CursorType::CURSOR_TYPE_CHOCOLATE ||
        aCursor == CursorType::CURSOR_TYPE_GLOVE || aCursor == CursorType::CURSOR_TYPE_MONEY_SIGN ||
        aCursor == CursorType::CURSOR_TYPE_WHEEELBARROW || aCursor == CursorType::CURSOR_TYPE_TREE_FOOD) {
        MouseDownWithTool(x, y, theClickCount, aCursor);
    } else if (IsPlantInCursor()) {
        MouseDownWithPlant(x, y, theClickCount);
    } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_SEEDPACKET) {
        if (!mBoardData.mPaused) {
            static_cast<SeedPacket *>(aHitResult.mObject)->MouseDown(x, y, theClickCount);
        }
    } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_NEXT_GARDEN) {
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
            mApp->mZenGarden->GotoNextGarden();
        } else if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
            mChallenge->TreeOfWisdomNextGarden();
        }
        mApp->PlaySample(Sexy::SOUND_TAP);
    } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_SHOVEL ||
               aHitResult.mObjectType ==
               GameObjectType::OBJECT_TYPE_WATERING_CAN ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_FERTILIZER
               ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_BUG_SPRAY
               ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_PHONOGRAPH
               ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_CHOCOLATE
               ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_GLOVE ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_MONEY_SIGN
               ||
               aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_WHEELBARROW
               ||
               aHitResult.mObjectType ==
               GameObjectType::OBJECT_TYPE_TREE_FOOD) {
        PickUpTool(aHitResult.mObjectType);
    } else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_PLANT) {
        static_cast<Plant *>(aHitResult.mObject)->MouseDown(x, y, theClickCount);
    }

    UpdateCursor();
}

// 0x412330
void Board::ClearCursor() {
    if (mAdvice->mDuration > 0) {
        if (mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_GRAVEBUSTERS_ON_GRAVES ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_LILYPAD_ON_WATER ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_TANGLEKELP_ON_WATER ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_SEASHROOM_ON_WATER ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_WRONG_ART_TYPE ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_NEED_POT ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_NOT_PASSED_LINE ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_REPEATERS ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_MELONPULT ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_SUNFLOWER ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_SPIKEWEED ||
            mBoardData.mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_KERNELPULT) {
            ClearAdvice(mBoardData.mHelpIndex);
        }
    }

    mCursorObject->mType = SeedType::SEED_NONE;
    mCursorObject->mCursorType = CursorType::CURSOR_TYPE_NORMAL;
    mCursorObject->mSeedBankIndex = -1;
    mCursorObject->mCoinID = CoinID::COINID_NULL;
    mCursorObject->mDuplicatorPlantID = PlantID::PLANTID_NULL;
    mCursorObject->mCobCannonPlantID = PlantID::PLANTID_NULL;
    mCursorObject->mGlovePlantID = PlantID::PLANTID_NULL;
    mApp->SetCursor(CURSOR_POINTER);
    mChallenge->ClearCursor();

    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER) {
        SetTutorialState(TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER);
    } else if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER || mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER) {
        if (!mSeedBank->mSeedPackets[1].CanPickUp()) {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER);
        } else {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
        }
    } else if (mBoardData.mTutorialState == TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER || mBoardData.mTutorialState == TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER) {
        if (!mSeedBank->mSeedPackets[1].CanPickUp()) {
            SetTutorialState(TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER);
        } else {
            SetTutorialState(TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER);
        }
    } else if (mBoardData.mTutorialState == TutorialState::TUTORIAL_SHOVEL_DIG) {
        SetTutorialState(TutorialState::TUTORIAL_SHOVEL_PICKUP);
    }
}

// 0x412490
bool Board::CanInteractWithBoardButtons() {
    if (mBoardData.mPaused || mApp->GetDialogCount() > 0) return false;

    if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL &&
        mCursorObject->mCursorType != CursorType::CURSOR_TYPE_HAMMER &&
        mCursorObject->mCursorType != CursorType::CURSOR_TYPE_COBCANNON_TARGET)
        return false;

    if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING) return false;

    return mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_OFF;
}

// 0x412540
void Board::MouseUp(const int x, const int y, const int theClickCount) {
    Widget::MouseUp(x, y, theClickCount);
    if (mIgnoreMouseUp) {
        mIgnoreMouseUp = false;
        return;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED && mChallenge->MouseUp(x, y) && theClickCount > 0)
        return;

    if (CanInteractWithBoardButtons() && theClickCount > 0) {
        if (mMenuButton->IsMouseOver() && !mApp->GetDialog(Dialogs::DIALOG_GAME_OVER) &&
            !mApp->GetDialog(Dialogs::DIALOG_LEVEL_COMPLETE)) {
            mMenuButton->mIsOver = false;
            mMenuButton->mIsDown = false;
            UpdateCursor();
            ClearCursor();
            if (mBoardData.mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_COMPLETED) {
                mApp->FinishZenGardenToturial();
            } else if (mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM && mApp->mGameMode != GAMEMODE_UPSELL) {
                mApp->PlaySample(Sexy::SOUND_PAUSE);
                mApp->DoNewOptions(false);
            } else {
                mApp->mBoardResult = BoardResult::BOARDRESULT_QUIT;
                mApp->DoBackToMain();
            }
        } else if (mStoreButton && mStoreButton->IsMouseOver()) {
            if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
                ClearAdviceImmediately();
                mApp->mZenGarden->OpenStore();
            } else if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
                mChallenge->TreeOfWisdomOpenStore();
            } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
                mChallenge->mChallengeState = ChallengeState::STATECHALLENGE_LAST_STAND_ONSLAUGHT;
                mStoreButton->mBtnNoDraw = true;
                mStoreButton->mDisabled = true;
                mBoardData.mZombieCountDown = 10;
                mBoardData.mZombieCountDownStart = 10;
            } else if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL) {
                /*if (mApp->mDRM)
                {
                    mApp->mDRM->BuyGame();
                }*/
                mApp->DoBackToMain();
            }
        }
    }
}

void Board::ShowCoinBank(const int theDuration) { mBoardData.mCoinBankFadeCount = theDuration; }

// 0x4127A0
void Board::Pause(const bool thePause) {
    if (mBoardData.mPaused == thePause) return;

    mBoardData.mPaused = thePause;
    if (thePause && mApp->mPlayerInfo->mCoins > 0) {
        ShowCoinBank();
    }

    if (!thePause || mApp->mGameScene != GameScenes::SCENE_LEVEL_INTRO) {
        mApp->mSoundSystem->GamePause(thePause);
        mApp->mMusic->GameMusicPause(thePause);
    }
}

// 0x412850
int Board::GetGraveStonesCount() {
    int aCount = 0;

    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE) {
            aCount++;
        }
    }

    return aCount;
}

// 0x412890
void Board::PickSpecialGraveStone() {
    GridItem *aGridItem = nullptr;
    GridItem *aPicks[MAX_GRAVE_STONES];
    int aPickCount = 0;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE) {
            TOD_ASSERT(aPickCount < MAX_GRAVE_STONES);
            aPicks[aPickCount] = aGridItem;
            aPickCount++;
        }
    }

    if (aPickCount > 0) {
        TodPickFromArray(aPicks, aPickCount)->mGridItemState = GridItemState::GRIDITEM_STATE_GRAVESTONE_SPECIAL;
    }
}

// 0x4128F0
void Board::SpawnZombiesFromPool() {
    if (mBoardData.mIceTrapCounter > 0) return;

    int aCount, aZombiePoints;
    if (mBoardData.mLevel == 21 || mBoardData.mLevel == 22 || mBoardData.mLevel == 31 || mBoardData.mLevel == 32) {
        aCount = 2;
        aZombiePoints = 3;
    } else if (mBoardData.mLevel == 23 || mBoardData.mLevel == 24 || mBoardData.
               mLevel == 25 || mBoardData.mLevel == 33 ||
               mBoardData.mLevel == 34 || mBoardData.mLevel == 35) {
        aCount = 3;
        aZombiePoints = 5;
    } else {
        aCount = 3;
        aZombiePoints = 7;
    }

    int aGridArrayCount = 0;
    TodWeightedGridArray aGridArray[MAX_POOL_GRID_SIZE];
    for (int aGridX = 5; aGridX < MAX_GRID_SIZE_X; aGridX++) {
        for (int aGridY = 2; aGridY <= 3; aGridY++) {
            aGridArray[aGridArrayCount].mX = aGridX;
            aGridArray[aGridArrayCount].mY = aGridY;
            aGridArray[aGridArrayCount].mWeight = 10000;
            aGridArrayCount++;
            TOD_ASSERT(aGridArrayCount <= MAX_POOL_GRID_SIZE);
        }
    }

    if (aGridArrayCount < 0) {
        aGridArrayCount = 0;
    }
    for (int i = 0; i < aCount; i++) {
        TodWeightedGridArray *aGrid = TodPickFromWeightedGridArray(aGridArray, aGridArrayCount);
        aGrid->mWeight = 0;

        const ZombieType aZombieType = PickGraveRisingZombieType();
        Zombie *aZombie = AddZombieInRow(aZombieType, aGrid->mY, mBoardData.mCurrentWave);
        if (aZombie == nullptr) {
            return;
        }

        aZombie->RiseFromGrave(aGrid->mX, aGrid->mY);
        aZombiePoints -= GetZombieDefinition(aZombieType).mZombieValue;
        if (aZombiePoints < 1) {
            aZombiePoints = 1;
        }
    }
}

// 0x412A90
void Board::SetupBungeeDrop(BungeeDropGrid *theBungeeDropGrid) {
    theBungeeDropGrid->mGridArrayCount = 0;
    for (int aGridX = 4; aGridX < MAX_GRID_SIZE_X; aGridX++) {
        for (int aGridY = 0; aGridY <= 4; aGridY++) {
            const int aCount = theBungeeDropGrid->mGridArrayCount;
            theBungeeDropGrid->mGridArray[aCount].mX = aGridX;
            theBungeeDropGrid->mGridArray[aCount].mY = aGridY;
            theBungeeDropGrid->mGridArray[aCount].mWeight = 10000;
            theBungeeDropGrid->mGridArrayCount++;
            TOD_ASSERT(
                static_cast<size_t>(theBungeeDropGrid->mGridArrayCount) <= std::size(theBungeeDropGrid->mGridArray)
            );
        }
    }
}

// 0x412B60
void Board::BungeeDropZombie(const BungeeDropGrid *theBungeeDropGrid, const ZombieType theZombieType) {
    TodWeightedGridArray *aGrid =
        TodPickFromWeightedGridArray(theBungeeDropGrid->mGridArray, theBungeeDropGrid->mGridArrayCount);
    aGrid->mWeight = 1;

    Zombie *aBungeeZombie = AddZombie(ZombieType::ZOMBIE_BUNGEE, mBoardData.mCurrentWave);
    Zombie *aZombie = AddZombie(theZombieType, mBoardData.mCurrentWave);
    TOD_ASSERT(aBungeeZombie && aZombie);

    aBungeeZombie->BungeeDropZombie(aZombie, aGrid->mX, aGrid->mY);
}

// 0x412C30
void Board::SpawnZombiesFromSky() {
    if (mBoardData.mIceTrapCounter > 0) return;

    int aCount, aZombiePoints;
    if (mBoardData.mLevel == 41 || mBoardData.mLevel == 42) {
        aCount = 2;
        aZombiePoints = 3;
    } else if (mBoardData.mLevel == 43 || mBoardData.mLevel == 44 || mBoardData.mLevel == 45) {
        aCount = 3;
        aZombiePoints = 5;
    } else {
        aCount = 3;
        aZombiePoints = 7;
    }

    BungeeDropGrid aBungeeDropGrid;
    SetupBungeeDrop(&aBungeeDropGrid);
    if (aCount > aBungeeDropGrid.mGridArrayCount) {
        aCount = aBungeeDropGrid.mGridArrayCount;
    }

    if (aBungeeDropGrid.mGridArrayCount == 0 || aCount <= 0) return;

    for (int i = 0; i < aCount; i++) {
        const ZombieType aZombieType = PickGraveRisingZombieType();
        BungeeDropZombie(&aBungeeDropGrid, aZombieType);
        aZombiePoints -= GetZombieDefinition(aZombieType).mZombieValue;
        if (aZombiePoints < 1) {
            aZombiePoints = 1;
        }
    }
}

// 0x412CE0
void Board::SpawnZombiesFromGraves() {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
        return;

    if (StageHasRoof()) {
        SpawnZombiesFromSky();
    } else if (StageHasPool()) {
        SpawnZombiesFromPool();
        return;
    }

    //	int aZombiePoints = GetGraveStonesCount();
    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridItemType != GridItemType::GRIDITEM_GRAVESTONE || aGridItem->mGridItemCounter < 100) {
            continue;
        }
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER &&
            Rand(mBoardData.mNumWaves) > mBoardData.mCurrentWave) {
            continue;
        }

        const ZombieType aZombieType = PickGraveRisingZombieType();
        Zombie *aZombie = AddZombie(aZombieType, mBoardData.mCurrentWave);
        if (aZombie == nullptr) {
            return;
        }

        aZombie->RiseFromGrave(aGridItem->mGridX, aGridItem->mGridY);
        /*
        aZombiePoints -= GetZombieDefinition(aZombieType).mZombieValue;
        if (aZombieType < 1)
        {
            aZombiePoints = 1;
        }
        */
    }
}

// 0x412E30
int Board::TotalZombiesHealthInWave(const int theWaveIndex) {
    int aTotalHealth = 0;
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mFromWave == theWaveIndex && !aZombie->mMindControlled && !aZombie->IsDeadOrDying() &&
            aZombie->mZombieType != ZombieType::ZOMBIE_BUNGEE && aZombie->mRelatedZombieID == ZombieID::ZOMBIEID_NULL) {
            aTotalHealth +=
                aZombie->mBodyHealth + aZombie->mHelmHealth + aZombie->mShieldHealth * 0.2f + aZombie->mFlyingHealth;
        }
    }
    return aTotalHealth;
}

// 0x412EE0
void Board::SpawnZombieWave() {
    mChallenge->SpawnZombieWave();
    if (mApp->IsBungeeBlitzLevel()) {
        BungeeDropGrid aBungeeDropGrid;
        SetupBungeeDrop(&aBungeeDropGrid);
        for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++) {
            const ZombieType aZombieType = mBoardData.mZombiesInWave[mBoardData.mCurrentWave][i];
            if (aZombieType == ZombieType::ZOMBIE_INVALID) break;

            if (aZombieType == ZombieType::ZOMBIE_BUNGEE || aZombieType == ZombieType::ZOMBIE_ZAMBONI) {
                AddZombie(aZombieType, mBoardData.mCurrentWave);
            } else {
                BungeeDropZombie(&aBungeeDropGrid, aZombieType);
            }
        }
    } else {
        TOD_ASSERT(
            mBoardData.mCurrentWave >= 0 && mBoardData.mCurrentWave < MAX_ZOMBIE_WAVES &&
            mBoardData.mCurrentWave < mBoardData.mNumWaves
        );
        for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++) {
            const ZombieType aZombieType = mBoardData.mZombiesInWave[mBoardData.mCurrentWave][i];
            if (aZombieType == ZombieType::ZOMBIE_INVALID) break;

            if (aZombieType == ZombieType::ZOMBIE_BOBSLED && !CanAddBobSled()) {
                for (int i = 0; i < MAX_ZOMBIE_FOLLOWERS; i++) {
                    AddZombie(
                        ZombieType::ZOMBIE_NORMAL,
                        mBoardData.mCurrentWave
                    ); // 生成 4 只普通僵尸以代替雪橇僵尸小队
                }
            } else {
                AddZombie(aZombieType, mBoardData.mCurrentWave);
            }
        }
    }

    if (mBoardData.mCurrentWave == mBoardData.mNumWaves - 1 && !mApp->IsContinuousChallenge()) {
        mBoardData.mRiseFromGraveCounter = 210;
    }
    if (IsFlagWave(mBoardData.mCurrentWave)) {
        mBoardData.mFlagRaiseCounter = FLAG_RAISE_TIME;
    }
    mBoardData.mCurrentWave++;
    mBoardData.mTotalSpawnedWaves++;
}

// 0x4130D0
void Board::UpdateGameObjects() {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        aPlant->Update();
    }

    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        aZombie->Update();
    }

    Projectile *aProjectile = nullptr;
    while (IterateProjectiles(aProjectile)) {
        aProjectile->Update();
    }

    Coin *aCoin = nullptr;
    while (IterateCoins(aCoin)) {
        aCoin->Update();
    }

    LawnMower *aLawnMower = nullptr;
    while (IterateLawnMowers(aLawnMower)) {
        aLawnMower->Update();
    }

    mCursorPreview->Update();
    mCursorObject->Update();

    for (int i = 0; i < mSeedBank->mNumPackets; i++) {
        mSeedBank->mSeedPackets[i].Update();
    }
}

// 0x413220
void Board::StopAllZombieSounds() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        aZombie->StopZombieSound();
    }
}

// 0x413260
//  GOTY @Patoke: 0x415BD0
int Board::GetSurvivalFlagsCompleted() {
    const int aWavesPerFlag = GetNumWavesPerFlag();
    const int aFlagsCompleted = mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() / aWavesPerFlag;
    int aCurrentWave = mBoardData.mCurrentWave;
    if (IsFlagWave(aCurrentWave - 1) && mBoardData.mBoardFadeOutCounter < 0 && !mBoardData.mNextSurvivalStageCounter) {
        aCurrentWave -= 1;
    }
    return aCurrentWave / aWavesPerFlag + aFlagsCompleted;
}

// 0x413320
void Board::SurvivalSaveScore() {
    if (!mApp->IsSurvivalMode()) return;

    const int aFlagsCompleted = GetSurvivalFlagsCompleted();
    int &aFlagsRecord = mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()];
    if (aFlagsCompleted > aFlagsRecord) {
        aFlagsRecord = aFlagsCompleted;
        mApp->WriteCurrentUserConfig();
    }
}

// 0x413390
void Board::PuzzleSaveStreak() {
    if (!mApp->IsEndlessIZombie(mApp->mGameMode) && !mApp->IsEndlessScaryPotter(mApp->mGameMode)) return;

    const int aStreak = mChallenge->mSurvivalStage + 1;
    int &aRecord = mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()];
    if (aStreak > aRecord) {
        aRecord = aStreak;
        mApp->WriteCurrentUserConfig();
    }
}

// 0x413400
void Board::ZombiesWon(Zombie *theZombie) {
    if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) return;

    ClearAdvice(AdviceType::ADVICE_NONE);
    mApp->mBoardResult = BoardResult::BOARDRESULT_LOST;

    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie == theZombie) continue;

        if (aZombie->GetZombieRect().mX < -50 || aZombie->mZombiePhase == ZombiePhase::PHASE_RISING_FROM_GRAVE ||
            aZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_RISING) {
            if ((aZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR ||
                 aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR) &&
                aZombie->IsDeadOrDying() && aZombie->mPosX < 140) {
                aZombie->DieNoLoot();
            }
        }
    }
    SurvivalSaveScore();

    SexyString aGameOverMsg;
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM) {
        aGameOverMsg = _S("[ZOMBIQUARIUM_DEATH_MESSAGE]");
    } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
        const SexyString aFlagStr =
            LawnApp::Pluralize(GetSurvivalFlagsCompleted(), _S("[ONE_FLAG]"), _S("[COUNT_FLAGS]"));
        aGameOverMsg = TodReplaceString(_S("[LAST_STAND_DEATH_MESSAGE]"), _S("{FLAGS}"), aFlagStr);
    } else if (LawnApp::IsEndlessIZombie(mApp->mGameMode) || LawnApp::IsEndlessScaryPotter(mApp->mGameMode)) {
        aGameOverMsg =
            TodReplaceNumberString(_S("[ENDLESS_PUZZLE_DEATH_MESSAGE]"), _S("{STREAK}"), mChallenge->mSurvivalStage);
    } else if (mApp->IsIZombieLevel()) {
        aGameOverMsg = _S("[I_ZOMBIE_DEATH_MESSAGE]");
    } else {
        mApp->mGameScene = GameScenes::SCENE_ZOMBIES_WON;
        if (theZombie) // 原版此处没有对 theZombie 进行空指针判断，但加上判断后便允许绕过僵尸而直接调用游戏失败
        {
            theZombie->WalkIntoHouse();
        }

        ClearAdvice(AdviceType::ADVICE_NONE);
        mCutScene->StartZombiesWon();
        FreezeEffectsForCutscene(true);
        TutorialArrowRemove();
        UpdateCursor();
        return;
    }

    const auto aGameOverDialog = new GameOverDialog(aGameOverMsg, true);
    mApp->AddDialog(Dialogs::DIALOG_GAME_OVER, aGameOverDialog);

    mApp->mMusic->StopAllMusic();
    StopAllZombieSounds();
    mApp->PlaySample(Sexy::SOUND_LOSEMUSIC);

    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIES_WON, true);
    Reanimation *aReanim = mApp->AddReanimation(
        -BOARD_OFFSET, 0, MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0),
        ReanimationType::REANIM_ZOMBIES_WON
    );
    aReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
    aReanim->GetTrackInstanceByName("fullscreen")->mTrackColor = Color::Black;
    aReanim->SetFramesForLayer("anim_screen");
}

// 0x4138D0
bool Board::IsFinalScaryPotterStage() {
    if (!mApp->IsScaryPotterLevel()) return false;

    if (mApp->IsAdventureMode()) {
        return mChallenge->mSurvivalStage == 2;
    }

    return !mApp->IsEndlessScaryPotter(mApp->mGameMode);
}

// 0x413920
bool Board::IsFinalSurvivalStage() {
    if (!mApp->IsSurvivalMode()) return false;

    const int aFlags = GetNumWavesPerSurvivalStage() * (mChallenge->mSurvivalStage + 1) / GetNumWavesPerFlag();
    if (mApp->IsSurvivalNormal(mApp->mGameMode)) {
        return aFlags >= 5;
    }
    if (mApp->IsSurvivalHard(mApp->mGameMode)) {
        return aFlags >= 10;
    }

    return false;
}

bool Board::IsLastStandFinalStage() {
    return mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND &&
           mChallenge->mSurvivalStage == LAST_STAND_FLAGS - 1;
}

// 0x4139E0
bool Board::IsSurvivalStageWithRepick() { return mApp->IsSurvivalMode() && !IsFinalSurvivalStage(); }

// 0x413A10
bool Board::IsLastStandStageWithRepick() {
    return mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND && !IsLastStandFinalStage();
}

// 0x413A40
bool Board::HasLevelAwardDropped() {
    return mBoardData.mLevelAwardSpawned || mBoardData.mNextSurvivalStageCounter > 0 ||
           mBoardData.mBoardFadeOutCounter >= 0;
}

// 0x413A70
void Board::UpdateSunSpawning() {
    if (StageIsNight() || HasLevelAwardDropped() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE || mApp->mGameMode == GameMode::GAMEMODE_UPSELL ||
        mApp->mGameMode == GameMode::GAMEMODE_INTRO || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mApp->IsIZombieLevel() ||
        mApp->IsScaryPotterLevel() || mApp->IsSquirrelLevel() || HasConveyorBeltSeedBank() ||
        mBoardData.mTutorialState == TutorialState::TUTORIAL_SLOT_MACHINE_PULL)
        return;

    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER ||
        mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER) {
        if (mPlants.mSize == 0) {
            return;
        }
    }

    mBoardData.mSunCountDown--;
    if (mBoardData.mSunCountDown != 0) return;

    mBoardData.mNumSunsFallen++;
    mBoardData.mSunCountDown =
        std::min(SUN_COUNTDOWN_MAX, SUN_COUNTDOWN + mBoardData.mNumSunsFallen * 10) + Rand(SUN_COUNTDOWN_RANGE);
    const CoinType aSunType =
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_SUNNY_DAY ? CoinType::COIN_LARGESUN : CoinType::COIN_SUN;
    AddCoin(RandRangeInt(100, 649), 60, aSunType, CoinMotion::COIN_MOTION_FROM_SKY);
}

// 0x413C00
void Board::NextWaveComing() {
    if (mBoardData.mCurrentWave + 1 == mBoardData.mNumWaves) {
        if (!IsSurvivalStageWithRepick() && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_LAST_STAND &&
            !mApp->IsContinuousChallenge()) {
            mApp->AddReanimation(
                0, 30, MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0), ReanimationType::REANIM_FINAL_WAVE
            );
            mBoardData.mFinalWaveSoundCounter = 60;
        }
    }

    if (mBoardData.mCurrentWave == 0) {
        mApp->PlaySample(Sexy::SOUND_AWOOGA);
    } else if (
        (mApp->IsWhackAZombieLevel() && mBoardData.mCurrentWave == mBoardData.
         mNumWaves - 1) || IsFlagWave(
            mBoardData.mCurrentWave)) {
        mApp->PlaySample(Sexy::SOUND_SIREN);
    }
}

// 0x413D00
void Board::UpdateZombieSpawning() {
    if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mGameMode == GameMode::GAMEMODE_INTRO) return;

    if (mBoardData.mFinalWaveSoundCounter > 0) {
        mBoardData.mFinalWaveSoundCounter--;
        if (mBoardData.mFinalWaveSoundCounter == 0) {
            mApp->PlaySample(Sexy::SOUND_FINALWAVE);
        }
    }

    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER ||
        mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER ||
        mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER ||
        mBoardData.mTutorialState == TutorialState::TUTORIAL_SLOT_MACHINE_PULL)
        return;

    if (HasLevelAwardDropped()) return;

    if (mBoardData.mRiseFromGraveCounter > 0) {
        mBoardData.mRiseFromGraveCounter--;
        if (mBoardData.mRiseFromGraveCounter == 0) {
            SpawnZombiesFromGraves();
        }
    }

    if (mBoardData.mHugeWaveCountDown > 0) {
        mBoardData.mHugeWaveCountDown--;
        if (mBoardData.mHugeWaveCountDown == 0) {
            ClearAdvice(AdviceType::ADVICE_HUGE_WAVE);
            NextWaveComing();
            mBoardData.mZombieCountDown = 1;
        } else {
            if (mBoardData.mHugeWaveCountDown == 725) {
                mApp->PlaySample(Sexy::SOUND_HUGE_WAVE);
            } else {
                if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK ||
                    mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES ||
                    mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST ||
                    mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF) {
                    if (mBoardData.mHugeWaveCountDown == 400) {
                        mApp->mMusic->StartBurst();
                    }
                } else if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS) {
                    if (mBoardData.mHugeWaveCountDown == 700) {
                        mApp->mMusic->StartBurst();
                    }
                }
                return;
            }
        }
    }

    if (mChallenge->UpdateZombieSpawning()) return;

    if (mBoardData.mCurrentWave == mBoardData.mNumWaves) {
        if (IsFinalSurvivalStage()) {
            return;
        }
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
            return;
        }
        if (!mApp->IsSurvivalMode() && !mApp->IsContinuousChallenge()) {
            return;
        }
    }

    mBoardData.mZombieCountDown--;
    if (mBoardData.mCurrentWave == mBoardData.mNumWaves && mApp->IsSurvivalMode()) {
        if (mBoardData.mZombieCountDown == 0) {
            FadeOutLevel();
        }
        return;
    }

    if (mBoardData.mZombieCountDown > 200 && mBoardData.mZombieCountDownStart - mBoardData.mZombieCountDown > 400 &&
        TotalZombiesHealthInWave(mBoardData.mCurrentWave - 1) <= mBoardData.mZombieHealthToNextWave) {
        mBoardData.mZombieCountDown = 200;
    }
    if (mBoardData.mZombieCountDown == 5) {
        if (IsFlagWave(mBoardData.mCurrentWave)) {
            ClearAdviceImmediately();
            DisplayAdviceAgain(
                _S("[ADVICE_HUGE_WAVE]"), MessageStyle::MESSAGE_STYLE_HUGE_WAVE, AdviceType::ADVICE_HUGE_WAVE
            );
            mBoardData.mHugeWaveCountDown = 750;
            return;
        }
        NextWaveComing();
    }
    if (mBoardData.mZombieCountDown == 0) {
        SpawnZombieWave();
        mBoardData.mZombieHealthWaveStart = TotalZombiesHealthInWave(mBoardData.mCurrentWave - 1);

        if (mBoardData.mCurrentWave == mBoardData.mNumWaves && mApp->IsSurvivalMode()) {
            mBoardData.mZombieHealthToNextWave = 0;
            mBoardData.mZombieCountDown = ZOMBIE_COUNTDOWN_BEFORE_REPICK + 1;
        } else if (IsFlagWave(mBoardData.mCurrentWave) && (mApp->IsWallnutBowlingLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)) {
            mBoardData.mZombieHealthToNextWave = 0;
            mBoardData.mZombieCountDown = ZOMBIE_COUNTDOWN_BEFORE_FLAG;
        } else {
            mBoardData.mZombieHealthToNextWave = RandRangeFloat(0.5f, 0.65f) * mBoardData.mZombieHealthWaveStart;
            if (mApp->IsLittleTroubleLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN ||
                mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
                mBoardData.mZombieCountDown = 750;
            } else {
                mBoardData.mZombieCountDown = ZOMBIE_COUNTDOWN + Rand(ZOMBIE_COUNTDOWN_RANGE);
            }
        }
        mBoardData.mZombieCountDownStart = mBoardData.mZombieCountDown;
    }
}

// 0x414100
void Board::UpdateIce() {
    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        if (mBoardData.mIceTimer[aRow]) {
            mBoardData.mIceTimer[aRow]--;
            TodParticleSystem *aParticleIce = mApp->ParticleTryToGet(mBoardData.mIceParticleID[aRow]);
            if (mBoardData.mIceTimer[aRow] == 0) {
                mBoardData.mIceMinX[aRow] = BOARD_ICE_START;
                if (aParticleIce) {
                    aParticleIce->ParticleSystemDie();
                }
            } else {
                const float aPosX = mBoardData.mIceMinX[aRow];
                const float aPosY = GridToPixelY(8, aRow);
                if (aParticleIce) {
                    aParticleIce->SystemMove(aPosX, aPosY);
                } else {
                    const int aRenderPosition = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, aRow, 3);
                    aParticleIce =
                        mApp->AddTodParticle(aPosX, aPosY, aRenderPosition, ParticleEffect::PARTICLE_ICE_SPARKLE);
                    mBoardData.mIceParticleID[aRow] = mApp->ParticleGetID(aParticleIce);
                }
            }

            const int anAlpha = ClampInt(mBoardData.mIceTimer[aRow] / 10, 0, 255);
            aParticleIce->OverrideColor(nullptr, Color(255, 255, 255, anAlpha));
        }
    }
}

// 0x414270
void Board::UpdateProgressMeter() {
    if (mApp->IsFinalBossLevel()) {
        Zombie *aBoss = GetBossZombie();
        if (aBoss && !aBoss->IsDeadOrDying()) {
            mBoardData.mProgressMeterWidth = 150 * (aBoss->mBodyMaxHealth - aBoss->mBodyHealth) / aBoss->mBodyMaxHealth;
        } else {
            mBoardData.mProgressMeterWidth = 150;
        }
    } else if (mBoardData.mCurrentWave != 0) {
        // 更新旗帜升起倒计时
        if (mBoardData.mFlagRaiseCounter > 0) mBoardData.mFlagRaiseCounter--;

        int aTotalWidth = 150; // 可用于平均分配给每一小波的进度条总长度
        const int aNumWavesPerFlag = GetNumWavesPerFlag();
        // 本关卡中每相邻两个旗帜波之前的小波数量
        const bool aHasFlags = ProgressMeterHasFlags(); // 进度条标注旗帜时，旗帜波占用更长的进度条
        if (aHasFlags) {
            aTotalWidth -= 12 * mBoardData.mNumWaves / aNumWavesPerFlag;
            // 从每个旗帜波分割出 12 单位的长度
        }

        const int aWaveLength = aTotalWidth / (mBoardData.mNumWaves - 1);
        // 每一小波占用的进度条长度
        int aCurrentWaveLength = (mBoardData.mCurrentWave - 1) * aTotalWidth / (mBoardData.mNumWaves - 1);
        // 当前波开始时的进度条长度
        int aNextWaveLength =
            mBoardData.mCurrentWave * aTotalWidth / (mBoardData.mNumWaves - 1); // 下一波开始时的进度条长度
        if (aHasFlags) {
            const int anExtraLength = mBoardData.mCurrentWave / aNumWavesPerFlag * 12;
            // 归还已刷新的旗帜波分割的长度
            aCurrentWaveLength += anExtraLength;
            aNextWaveLength += anExtraLength;
        }

        // 根据倒计时初步计算当前波已经过的比例
        float aFraction = (mBoardData.mZombieCountDownStart - mBoardData.mZombieCountDown) /
                          static_cast<float>(mBoardData.mZombieCountDownStart);
        if (mBoardData.mZombieHealthToNextWave != -1) {
            // 取得本波僵尸的当前血量
            const int aHealthCurrent = TotalZombiesHealthInWave(mBoardData.mCurrentWave - 1);
            // 取得（本波开始时的僵尸总血量 - 下一波刷新时的僵尸总血量），即：本波刷新需要对僵尸造成的伤害
            int aDamageTarget = mBoardData.mZombieHealthWaveStart - mBoardData.mZombieHealthToNextWave;
            // 开始时的血量 - 刷新时的血量
            if (aDamageTarget < 1) {
                aDamageTarget = 1; // 需要的伤害至少为 1
            }
            // 再次以刷新血量计算一次当前波已经过的比例
            // 血量比例 = [目标伤害 - (当前血量 - 刷新血量)] / 目标伤害 = (目标伤害 - 仍需造成的伤害) / 目标伤害 =
            // 当前伤害 / 目标伤害
            const float aHealthFraction = (aDamageTarget - aHealthCurrent + mBoardData.mZombieHealthToNextWave) /
                                          static_cast<float>(aDamageTarget);
            // 最终比例取上述二者的较大值
            aFraction = std::max(aHealthFraction, aFraction);
        }

        // 计算当前应当的进度条长度，并将长度的范围限定在 [1, 150] 之间
        const int aLength =
            ClampInt(aCurrentWaveLength + FloatRoundToInt((aNextWaveLength - aCurrentWaveLength) * aFraction), 1, 150);
        // 取得当前实际与理论的进度条长度之差
        const int aDelta = aLength - mBoardData.mProgressMeterWidth;
        // 当差值不超过一波的长度时，每 20cs 调整一次长度；否则，每 5cs 调整一次长度
        if ((aDelta > aWaveLength && (mBoardData.mMainCounter % 5 == 0)) ||
            (aDelta > 0 && (mBoardData.mMainCounter % 20 == 0))) {
            mBoardData.mProgressMeterWidth++;
        }
    }
}

// 0x4144E0
void Board::UpdateTutorial() {
    if (mBoardData.mTutorialTimer > 0) mBoardData.mTutorialTimer--;

    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER &&
        mBoardData.mTutorialTimer == 0) {
        DisplayAdvice(
            _S("[ADVICE_CLICK_PEASHOOTER]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE
        );
        TutorialArrowShow(mSeedBank->mX + mSeedBank->mSeedPackets[1].mX, mSeedBank->mY + mSeedBank->mSeedPackets[1].mY);
        mBoardData.mTutorialTimer = -1;
    } else if (mBoardData.mTutorialState ==
               TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER ||
               mBoardData.mTutorialState ==
               TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER ||
               mBoardData.mTutorialState ==
               TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER) {
        if (mBoardData.mTutorialTimer == 0) {
            DisplayAdvice(
                _S("[ADVICE_PLANT_SUNFLOWER2]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE
            );
            mBoardData.mTutorialTimer = -1;
        } else if (mBoardData.mZombieCountDown == 750 && mBoardData.mCurrentWave == 0) {
            DisplayAdvice(
                _S("[ADVICE_PLANT_SUNFLOWER3]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE
            );
        }
    } else if (mBoardData.mTutorialState ==
               TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER ||
               mBoardData.mTutorialState ==
               TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER ||
               mBoardData.mTutorialState ==
               TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER) {
        if (mBoardData.mTutorialTimer == 0) {
            DisplayAdvice(
                _S("[ADVICE_PLANT_SUNFLOWER5]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER,
                AdviceType::ADVICE_PLANT_SUNFLOWER5
            );
            mBoardData.mTutorialTimer = -1;
        }
    }

    // 冒险模式初期关卡，检测到向日葵数量小于 3 时，进入“更多向日葵”的教程
    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel >= 3 && mBoardData.mLevel != 5 &&
        mBoardData.mLevel <= 7 && mBoardData.mTutorialState == TutorialState::TUTORIAL_OFF &&
        mBoardData.mCurrentWave >= 5 && !gShownMoreSunTutorial && mSeedBank->mSeedPackets[1].CanPickUp() &&
        CountPlantByType(SeedType::SEED_SUNFLOWER) < 3) {
        TOD_ASSERT(!ChooseSeedsOnCurrentLevel());
        DisplayAdvice(
            _S("[ADVICE_PLANT_SUNFLOWER4]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY, AdviceType::ADVICE_NONE
        );
        gShownMoreSunTutorial = true;
        SetTutorialState(TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER);
        mBoardData.mTutorialTimer = 500;
    }
}

// 0x414CB0
void Board::SetTutorialState(const TutorialState theTutorialState) {
    switch (theTutorialState) {
    case TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER:
        if (mPlants.mSize == 0) {
            const float aPosX = mSeedBank->mX + mSeedBank->mSeedPackets[0].mX;
            const float aPosY = mSeedBank->mY + mSeedBank->mSeedPackets[0].mY;
            TutorialArrowShow(aPosX, aPosY);
            DisplayAdvice(
                _S("[ADVICE_CLICK_SEED_PACKET]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY,
                AdviceType::ADVICE_NONE
            );
        } else {
            DisplayAdvice(
                _S("[ADVICE_ENOUGH_SUN]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE
            );
            mBoardData.mTutorialTimer = 400;
        }
        break;

    case TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER:
        mBoardData.mTutorialTimer = -1;
        TutorialArrowRemove();
        if (mPlants.mSize == 0) {
            DisplayAdvice(
                _S("[ADVICE_CLICK_ON_GRASS]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE
            );
        } else {
            ClearAdvice(AdviceType::ADVICE_NONE);
        }
        break;

    case TutorialState::TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER:
        DisplayAdvice(
            _S("[ADVICE_PLANTED_PEASHOOTER]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE
        );
        mBoardData.mSunCountDown = 400;
        break;

    case TutorialState::TUTORIAL_LEVEL_1_COMPLETED:
        DisplayAdvice(
            _S("[ADVICE_ZOMBIE_ONSLAUGHT]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1, AdviceType::ADVICE_NONE
        );
        mBoardData.mZombieCountDown = 99;
        mBoardData.mZombieCountDownStart = mBoardData.mZombieCountDown;
        break;

    case TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER:
    case TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER:
        TutorialArrowShow(mSeedBank->mX + mSeedBank->mSeedPackets[1].mX, mSeedBank->mX + mSeedBank->mSeedPackets[1].mX);
        break;

    case TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER:
    case TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER:
    case TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER:
    case TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER: TutorialArrowRemove(); break;

    case TutorialState::TUTORIAL_LEVEL_2_COMPLETED:
        if (mBoardData.mCurrentWave == 0) {
            mBoardData.mZombieCountDown = 999;
            mBoardData.mZombieCountDownStart = mBoardData.mZombieCountDown;
        }
        break;

    case TutorialState::TUTORIAL_SLOT_MACHINE_PULL:
        DisplayAdvice(
            _S("[ADVICE_SLOT_MACHINE_PULL]"), MessageStyle::MESSAGE_STYLE_SLOT_MACHINE,
            AdviceType::ADVICE_SLOT_MACHINE_PULL
        );
        break;

    case TutorialState::TUTORIAL_SLOT_MACHINE_COMPLETED: ClearAdvice(AdviceType::ADVICE_SLOT_MACHINE_PULL); break;

    case TutorialState::TUTORIAL_SHOVEL_PICKUP: {
        DisplayAdvice(_S("[ADVICE_CLICK_SHOVEL]"), MessageStyle::MESSAGE_STYLE_HINT_STAY, AdviceType::ADVICE_NONE);
        const Rect aShovelButtonRect = GetShovelButtonRect();
        const int aPosX = aShovelButtonRect.mX + aShovelButtonRect.mWidth / 2 - 25;
        const int aPosY = aShovelButtonRect.mY + aShovelButtonRect.mHeight - 65;
        TutorialArrowShow(aPosX, aPosY);
        break;
    }

    case TutorialState::TUTORIAL_SHOVEL_DIG:
        DisplayAdvice(_S("[ADVICE_CLICK_PLANT]"), MessageStyle::MESSAGE_STYLE_HINT_STAY, AdviceType::ADVICE_NONE);
        TutorialArrowRemove();
        break;

    case TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING:
        DisplayAdvice(_S("[ADVICE_KEEP_DIGGING]"), MessageStyle::MESSAGE_STYLE_HINT_STAY, AdviceType::ADVICE_NONE);
        break;

    case TutorialState::TUTORIAL_SHOVEL_COMPLETED:
        ClearAdvice(AdviceType::ADVICE_NONE);
        mCutScene->mCutsceneTime = 1500;
        mCutScene->mCrazyDaveDialogStart = 2410;
        break;

    default: break;
    }

    mBoardData.mTutorialState = theTutorialState;
}

// 0x415920
void Board::UpdateGame() {
    UpdateGameObjects();
    if (StageHasFog() && mBoardData.mFogBlownCountDown > 0) {
        const float aMaxFogOffset = 1065.0f - LeftFogColumn() * 80.0f;
        if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO) {
            mBoardData.mFogOffset = TodAnimateCurveFloat(
                200, 0, mBoardData.mFogBlownCountDown, aMaxFogOffset, 0, TodCurves::CURVE_EASE_OUT
            );
        } else if (mBoardData.mFogBlownCountDown < 2000) {
            mBoardData.mFogOffset = TodAnimateCurveFloat(
                2000, 0, mBoardData.mFogBlownCountDown, aMaxFogOffset, 0, TodCurves::CURVE_EASE_OUT
            );
        } else if (mBoardData.mFogOffset < aMaxFogOffset) {
            mBoardData.mFogOffset = TodAnimateCurveFloat(
                -5, aMaxFogOffset, mBoardData.mFogOffset * 1.1f, 0, aMaxFogOffset, TodCurves::CURVE_LINEAR
            );
        }
    }

    if (mApp->mGameScene != GameScenes::SCENE_PLAYING && !mCutScene->ShouldRunUpsellBoard()) return;

    mBoardData.mMainCounter++;
    UpdateSunSpawning();
    UpdateZombieSpawning();
    UpdateIce();
    if (mBoardData.mIceTrapCounter > 0) {
        mBoardData.mIceTrapCounter--;
        if (mBoardData.mIceTrapCounter == 0) {
            TodParticleSystem *aPoolSparklyParticle = mApp->ParticleTryToGet(mBoardData.mPoolSparklyParticleID);
            if (aPoolSparklyParticle) {
                aPoolSparklyParticle->mDontUpdate = false;
            }
        }
    }

    if (mBoardData.mFogBlownCountDown > 0) {
        mBoardData.mFogBlownCountDown--;
    }

    if (mBoardData.mMainCounter == 1 && mApp->IsFirstTimeAdventureMode()) {
        if (mBoardData.mLevel == 1) {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER);
        } else if (mBoardData.mLevel == 2) {
            SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
            DisplayAdvice(
                _S("[ADVICE_PLANT_SUNFLOWER1]"), MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE
            );
            mBoardData.mTutorialTimer = 500;
        }
    }

    UpdateProgressMeter();
}

// 0x415D40
void Board::Update() {
    TodHesitationBracket aHesitation("Board::Update");

    Widget::Update();
    MarkDirty();

    mCutScene->Update();
    UpdateMousePosition();
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        mApp->mZenGarden->ZenGardenUpdate();
    }
    if (IsScaryPotterDaveTalking()) {
        mApp->UpdateCrazyDave();
    }

    if (mBoardData.mPaused) {
        mChallenge->Update();
        mCursorPreview->mVisible = false;
        mCursorObject->mVisible = false;
        return;
    }

    const bool aDisabled = !CanInteractWithBoardButtons() || mIgnoreMouseUp;
    if (!mMenuButton->mBtnNoDraw) {
        mMenuButton->mDisabled = aDisabled;
    }
    mMenuButton->Update();
    if (mStoreButton) {
        mStoreButton->mDisabled = aDisabled;
        mStoreButton->Update();
    }

    mApp->mEffectSystem->Update();
    mAdvice->Update();
    UpdateTutorial();

    if (mBoardData.mCobCannonCursorDelayCounter > 0) {
        mBoardData.mCobCannonCursorDelayCounter--;
    }
    if (mBoardData.mOutOfMoneyCounter > 0) {
        mBoardData.mOutOfMoneyCounter--;
    }
    if (mBoardData.mShakeCounter > 0) {
        mBoardData.mShakeCounter--;
        if (mBoardData.mShakeCounter == 0) {
            mX = 0;
            mY = 0;
        } else {
            if (!Rand(3)) {
                mBoardData.mShakeAmountX = -mBoardData.mShakeAmountX;
            }
            mX = TodAnimateCurve(12, 0, mBoardData.mShakeCounter, 0, mBoardData.mShakeAmountX, TodCurves::CURVE_BOUNCE);
            mY = TodAnimateCurve(12, 0, mBoardData.mShakeCounter, 0, mBoardData.mShakeAmountY, TodCurves::CURVE_BOUNCE);
        }
    }
    if (mBoardData.mCoinBankFadeCount > 0 && mApp->GetDialog(Dialogs::DIALOG_PURCHASE_PACKET_SLOT) == nullptr) {
        mBoardData.mCoinBankFadeCount--;
    }
    UpdateLayers();

    if (mBoardData.mTimeStopCounter > 0) return;

    mBoardData.mEffectCounter++;
    if (StageHasPool() && !mBoardData.mIceTrapCounter && mApp->mGameScene != GameScenes::SCENE_ZOMBIES_WON &&
        !mCutScene->IsSurvivalRepick()) {
        mApp->mPoolEffect->mPoolCounter++;
    }
    if (mBoardData.mBackground == BackgroundType::BACKGROUND_3_POOL &&
        mBoardData.mPoolSparklyParticleID == ParticleSystemID::PARTICLESYSTEMID_NULL && mBoardData.mDrawCount > 0) {
        const int aRenderPosition = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, 2, 0);
        TodParticleSystem *aPoolParticle =
            mApp->AddTodParticle(450, 295, aRenderPosition, ParticleEffect::PARTICLE_POOL_SPARKLY);
        mBoardData.mPoolSparklyParticleID = mApp->ParticleGetID(aPoolParticle);
    }

    UpdateGridItems();
    UpdateFwoosh();
    UpdateGame();
    UpdateFog();
    mChallenge->Update();
    UpdateLevelEndSequence();
    mBoardData.mPrevMouseX = mApp->mWidgetManager->mLastMouseX;
    mBoardData.mPrevMouseY = mApp->mWidgetManager->mLastMouseY;
}

// 0x416080
//  GOTY @Patoke: 0x418940
void Board::UpdateLayers() {
    if (mWidgetManager) {
        mWidgetManager->MarkAllDirty();

        for (auto anIter = mApp->mDialogList.begin(); anIter != mApp->mDialogList.end(); ++anIter) {
            Dialog *aDialog = *anIter;
            mWidgetManager->BringToFront(aDialog);
            aDialog->MarkDirty();
        }
    }
}

// 0x416110
bool Board::RowCanHaveZombies(const int theRow) {
    if (theRow < 0 || theRow >= MAX_GRID_SIZE_Y) return false;

    return (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED && theRow <= 4) ||
           mBoardData.mPlantRow[theRow] != PlantRowType::PLANTROW_DIRT;
}

int Board::GetIceZPos(const int theRow) { return MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, theRow, 2); }

// 0x416140
void Board::DrawIce(Graphics *g, const int theGridY) {
    const int aPosY = GridToPixelY(8, theGridY) + 20;
    const int aHeight = Sexy::IMAGE_ICE->GetHeight();
    const int aWidth = Sexy::IMAGE_ICE->GetWidth();
    const int anAlpha = ClampInt(255 * mBoardData.mIceTimer[theGridY] / 10, 0, 255);
    if (anAlpha < 255) {
        g->SetColorizeImages(true);
        g->SetColor(Color(255, 255, 255, anAlpha));
    }

    int aBeginningX = mBoardData.mIceMinX[theGridY] + 13, aDeltaX;
    for (int aPosX = aBeginningX; aPosX < BOARD_WIDTH; aPosX += aDeltaX) {
        if (aPosX == aBeginningX) {
            aDeltaX = (BOARD_WIDTH - aBeginningX) % aWidth;
            if (!aDeltaX) aDeltaX = aWidth;
        } else aDeltaX = aWidth;
        Rect aRepeatSrcRect(aWidth - aDeltaX, 0, aDeltaX, aHeight);
        Rect aRepeatDstRect(aPosX, aPosY, aDeltaX, aHeight);
        g->DrawImage(Sexy::IMAGE_ICE, aRepeatDstRect, aRepeatSrcRect);
    }
    g->DrawImage(Sexy::IMAGE_ICE_CAP, mBoardData.mIceMinX[theGridY], aPosY);
    g->SetColorizeImages(false);
}

// 0x416290
void Board::DrawBackdrop(Graphics *g) {
    Image *aBgImage = nullptr;
    switch (mBoardData.mBackground) {
    case BackgroundType::BACKGROUND_1_DAY:           aBgImage = Sexy::IMAGE_BACKGROUND1; break;
    case BackgroundType::BACKGROUND_2_NIGHT:         aBgImage = Sexy::IMAGE_BACKGROUND2; break;
    case BackgroundType::BACKGROUND_3_POOL:          aBgImage = Sexy::IMAGE_BACKGROUND3; break;
    case BackgroundType::BACKGROUND_4_FOG:           aBgImage = Sexy::IMAGE_BACKGROUND4; break;
    case BackgroundType::BACKGROUND_5_ROOF:          aBgImage = Sexy::IMAGE_BACKGROUND5; break;
    case BackgroundType::BACKGROUND_6_BOSS:          aBgImage = Sexy::IMAGE_BACKGROUND6BOSS; break;
    case BackgroundType::BACKGROUND_MUSHROOM_GARDEN: aBgImage = Sexy::IMAGE_BACKGROUND_MUSHROOMGARDEN; break;
    case BackgroundType::BACKGROUND_GREENHOUSE:      aBgImage = Sexy::IMAGE_BACKGROUND_GREENHOUSE; break;
    case BackgroundType::BACKGROUND_ZOMBIQUARIUM:    aBgImage = Sexy::IMAGE_AQUARIUM1; break;
    case BackgroundType::BACKGROUND_TREEOFWISDOM:    aBgImage = nullptr; break;
    default:                                         TOD_ASSERT(); break;
    }

    if (mBoardData.mLevel == 1 && mApp->IsFirstTimeAdventureMode()) {
        g->DrawImage(Sexy::IMAGE_BACKGROUND1UNSODDED, -BOARD_OFFSET, 0);
        const int aWidth = TodAnimateCurve(
            0, 1000, mBoardData.mSodPosition, 0, Sexy::IMAGE_SOD1ROW->GetWidth(), TodCurves::CURVE_LINEAR
        );
        const Rect aSrcRect(0, 0, aWidth, Sexy::IMAGE_SOD1ROW->GetHeight());
        g->DrawImage(Sexy::IMAGE_SOD1ROW, 239 - BOARD_OFFSET, 265, aSrcRect);
    } else if (((mBoardData.mLevel == 2 || mBoardData.mLevel == 3) && mApp->IsFirstTimeAdventureMode()) || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED) {
        g->DrawImage(Sexy::IMAGE_BACKGROUND1UNSODDED, -BOARD_OFFSET, 0);
        g->DrawImage(Sexy::IMAGE_SOD1ROW, 239 - BOARD_OFFSET, 265);
        const int aWidth = TodAnimateCurve(
            0, 1000, mBoardData.mSodPosition, 0, Sexy::IMAGE_SOD3ROW->GetWidth(), TodCurves::CURVE_LINEAR
        );
        const Rect aSrcRect(0, 0, aWidth, Sexy::IMAGE_SOD3ROW->GetHeight());
        g->DrawImage(Sexy::IMAGE_SOD3ROW, 235 - BOARD_OFFSET, 149, aSrcRect);
    } else if (mBoardData.mLevel == 4 && mApp->IsFirstTimeAdventureMode()) {
        g->DrawImage(Sexy::IMAGE_BACKGROUND1UNSODDED, -BOARD_OFFSET, 0);
        g->DrawImage(Sexy::IMAGE_SOD3ROW, 235 - BOARD_OFFSET, 149);
        const int aWidth = TodAnimateCurve(0, 1000, mBoardData.mSodPosition, 0, 773, TodCurves::CURVE_LINEAR);
        const Rect aSrcRect(232, 0, aWidth, Sexy::IMAGE_BACKGROUND1->GetHeight());
        g->DrawImage(Sexy::IMAGE_BACKGROUND1, 232 - BOARD_OFFSET, 0, aSrcRect);
    } else if (aBgImage) {
        if (aBgImage == Sexy::IMAGE_BACKGROUND_MUSHROOMGARDEN || aBgImage == Sexy::IMAGE_BACKGROUND_GREENHOUSE ||
            aBgImage == Sexy::IMAGE_AQUARIUM1) {
            g->DrawImage(aBgImage, 0, 0);
        } else {
            g->DrawImage(aBgImage, -BOARD_OFFSET, 0);
        }
    }

    if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) {
        DrawHouseDoorBottom(g);
    }
    if (StageHasPool()) {
        mApp->mPoolEffect->PoolEffectDraw(g, StageIsNight());
    }
    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER) {
        Graphics aClipG(*g);
        aClipG.SetColorizeImages(true);
        aClipG.SetColor(GetFlashingColor(mBoardData.mMainCounter, 75));
        aClipG.DrawImage(Sexy::IMAGE_SOD1ROW, 239 - BOARD_OFFSET, 265);
        aClipG.SetColorizeImages(false);
    }
    mChallenge->DrawBackdrop(g);
    if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && StageHasGraveStones()) {
        g->DrawImage(Sexy::IMAGE_NIGHT_GRAVE_GRAPHIC, 1092, 40);
    }
}

// 0x416690
bool RenderItemSortFunc(const RenderItem &theItem1, const RenderItem &theItem2) {
    if (theItem1.mZPos == theItem2.mZPos) {
        return theItem1.mGameObject < theItem2.mGameObject;
    }

    return theItem1.mZPos < theItem2.mZPos;
}

// 0x4166C0
void Board::AddBossRenderItem(RenderItem *theRenderList, int &theCurRenderItem, Zombie *theBossZombie) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    int aBackLegRow = 1;
    int aFrontLegRow = 3;
    int aBackArmRow = 4;
    if (theBossZombie->IsDeadOrDying()) {
        aBackArmRow = 1;
    } else if (theBossZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_STOMPING) {
        const Reanimation *aBossReanim = mApp->ReanimationTryToGet(theBossZombie->mBodyReanimID);
        if (aBossReanim->mAnimTime > 0.25f && aBossReanim->mAnimTime < 0.75f) {
            if (theBossZombie->mTargetRow == 1) {
                aBackLegRow = 2;
            } else if (theBossZombie->mTargetRow == 3) {
                aFrontLegRow = 4;
            }
        }
    }

    RenderItem *aItem = &theRenderList[theCurRenderItem];
    aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
    aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, aBackLegRow, 2);
    aItem->mBossPart = BossPart::BOSS_PART_BACK_LEG;
    theCurRenderItem++;
    aItem = &theRenderList[theCurRenderItem];
    aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
    aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, aFrontLegRow, 2);
    aItem->mBossPart = BossPart::BOSS_PART_FRONT_LEG;
    theCurRenderItem++;
    aItem = &theRenderList[theCurRenderItem];
    aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
    aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, 4, 2);
    aItem->mBossPart = BossPart::BOSS_PART_MAIN;
    theCurRenderItem++;
    aItem = &theRenderList[theCurRenderItem];
    aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
    aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, aBackArmRow, 3);
    aItem->mBossPart = BossPart::BOSS_PART_BACK_ARM;
    theCurRenderItem++;

    const Reanimation *aBallReanim = mApp->ReanimationTryToGet(theBossZombie->mBossFireBallReanimID);
    if (aBallReanim) {
        RenderItem *aItem = &theRenderList[theCurRenderItem];
        aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
        aItem->mZPos = aBallReanim->mRenderOrder;
        aItem->mBossPart = BossPart::BOSS_PART_FIREBALL;
        theCurRenderItem++;
    }
}

/*
[[maybe_unused]]
static inline void AddGameObjectRenderItem(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType
theRenderObjectType, GameObject* theGameObject)
{
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem& aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = theGameObject->mRenderOrder;
    aRenderItem.mGameObject = theGameObject;
    theCurRenderItem++;
}
*/

static inline void AddGameObjectRenderItemCursorPreview(
    RenderItem *theRenderList, int &theCurRenderItem, const RenderObjectType theRenderObjectType,
    GameObject *theGameObject
) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem &aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = theGameObject->mRenderOrder;
    aRenderItem.mGameObject = theGameObject;
    aRenderItem.mCursorPreview = static_cast<CursorPreview *>(theGameObject);

    theCurRenderItem++;
}

static inline void AddGameObjectRenderItemPlant(
    RenderItem *theRenderList, int &theCurRenderItem, const RenderObjectType theRenderObjectType,
    GameObject *theGameObject
) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem &aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = theGameObject->mRenderOrder;
    aRenderItem.mGameObject = theGameObject;
    aRenderItem.mPlant = static_cast<Plant *>(theGameObject);

    theCurRenderItem++;
}

static inline void AddGameObjectRenderItemZombie(
    RenderItem *theRenderList, int &theCurRenderItem, const RenderObjectType theRenderObjectType,
    GameObject *theGameObject
) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem &aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = theGameObject->mRenderOrder;
    aRenderItem.mGameObject = theGameObject;
    aRenderItem.mZombie = static_cast<Zombie *>(theGameObject);
    theCurRenderItem++;
}

static inline void AddGameObjectRenderItemProjectile(
    RenderItem *theRenderList, int &theCurRenderItem, const RenderObjectType theRenderObjectType,
    GameObject *theGameObject
) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem &aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = theGameObject->mRenderOrder;
    aRenderItem.mGameObject = theGameObject;
    aRenderItem.mProjectile = static_cast<Projectile *>(theGameObject);
    theCurRenderItem++;
}

static inline void AddGameObjectRenderItemCoin(
    RenderItem *theRenderList, int &theCurRenderItem, const RenderObjectType theRenderObjectType,
    GameObject *theGameObject
) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem &aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = theGameObject->mRenderOrder;
    aRenderItem.mGameObject = theGameObject;
    aRenderItem.mCoin = static_cast<Coin *>(theGameObject);
    theCurRenderItem++;
}

static inline void AddUIRenderItem(
    RenderItem *theRenderList, int &theCurRenderItem, const RenderObjectType theRenderObjectType, const int thePosZ
) {
    TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
    RenderItem &aRenderItem = theRenderList[theCurRenderItem];
    aRenderItem.mRenderObjectType = theRenderObjectType;
    aRenderItem.mZPos = thePosZ;
    aRenderItem.mGameObject = nullptr;
    theCurRenderItem++;
}

// 0x416880
void Board::DrawGameObjects(Graphics *g) {
    TodHesitationTrace("creating render list");

    RenderItem aRenderList[MAX_RENDER_ITEMS];
    int aRenderItemCount = 0;

    {
        Plant *aPlant = nullptr;
        while (IteratePlants(aPlant)) {
            if (aPlant->mOnBungeeState == PlantOnBungeeState::NOT_ON_BUNGEE) {
                AddGameObjectRenderItemPlant(
                    aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_PLANT, aPlant
                );

                if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && aPlant->mPottedPlantIndex != -1) {
                    RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                    aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PLANT_OVERLAY;
                    aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, 0, mY);
                    aRenderItem.mPlant = aPlant;
                    aRenderItemCount++;
                }

                if ((aPlant->mSeedType == SeedType::SEED_MAGNETSHROOM || aPlant->mSeedType == SeedType::SEED_GOLD_MAGNET
                    ) &&
                    aPlant->DrawMagnetItemsOnTop()) {
                    RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                    aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PLANT_MAGNET_ITEMS;
                    aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, -1);
                    aRenderItem.mPlant = aPlant;
                    aRenderItemCount++;
                }
            }
        }
    }
    {
        Coin *aCoin = nullptr;
        while (IterateCoins(aCoin)) {
            AddGameObjectRenderItemCoin(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_COIN, aCoin);
        }
    }
    {
        Zombie *aZombie = nullptr;
        while (IterateZombies(aZombie)) {
            if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) {
                AddBossRenderItem(aRenderList, aRenderItemCount, aZombie);
            } else {
                AddGameObjectRenderItemZombie(
                    aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_ZOMBIE, aZombie
                );

                if (aZombie->HasShadow()) {
                    RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                    aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_ZOMBIE_SHADOW;
                    aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, aZombie->mRow, 3);
                    aRenderItem.mZombie = aZombie;
                    aRenderItemCount++;
                }

                if (aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE) {
                    RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                    aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_ZOMBIE_BUNGEE_TARGET;
                    aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_PROJECTILE, aZombie->mRow, 1);
                    aRenderItem.mZombie = aZombie;
                    aRenderItemCount++;
                }
            }
        }
    }
    {
        Projectile *aProjectile = nullptr;
        while (IterateProjectiles(aProjectile)) {
            AddGameObjectRenderItemProjectile(
                aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_PROJECTILE, aProjectile
            );

            RenderItem &aRenderItem = aRenderList[aRenderItemCount];
            aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PROJECTILE_SHADOW;
            aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, aProjectile->mRow, 3);
            aRenderItem.mProjectile = aProjectile;
            aRenderItemCount++;
        }
    }
    {
        LawnMower *aLawnMower = nullptr;
        while (IterateLawnMowers(aLawnMower)) {
            RenderItem &aRenderItem = aRenderList[aRenderItemCount];
            aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_MOWER;
            aRenderItem.mZPos = aLawnMower->mRenderOrder;
            aRenderItem.mMower = aLawnMower;
            aRenderItemCount++;
        }
    }
    {
        TodParticleSystem *aParticle = nullptr;
        while (IterateParticles(aParticle)) {
            if (!aParticle->mIsAttachment) {
                RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PARTICLE;
                aRenderItem.mZPos = aParticle->mRenderOrder;
                aRenderItem.mParticleSytem = aParticle;
                aRenderItemCount++;
            }
        }
    }
    {
        Reanimation *aReanimation = nullptr;
        while (IterateReanimations(aReanimation)) {
            if (!aReanimation->mIsAttachment) {
                RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_REANIMATION;
                aRenderItem.mZPos = aReanimation->mRenderOrder;
                aRenderItem.mReanimation = aReanimation;
                aRenderItemCount++;
            }
        }
    }
    {
        GridItem *aGridItem = nullptr;
        while (IterateGridItems(aGridItem)) {
            RenderItem &aRenderItem = aRenderList[aRenderItemCount];
            aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_GRID_ITEM;
            aRenderItem.mZPos = aGridItem->mRenderOrder;
            aRenderItem.mGridItem = aGridItem;
            aRenderItemCount++;

            if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
                aGridItem->mGridItemType == GridItemType::GRIDITEM_STINKY) {
                RenderItem &aRenderItem = aRenderList[aRenderItemCount];
                aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_GRID_ITEM_OVERLAY;
                aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, 0, aGridItem->mPosY - 30.0f);
                aRenderItem.mGridItem = aGridItem;
                aRenderItemCount++;
            }
        }
    }
    for (int i = 0; i < MAX_GRID_SIZE_Y; i++) {
        if (mBoardData.mIceTimer[i]) {
            RenderItem &aRenderItem = aRenderList[aRenderItemCount];
            aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_ICE;
            aRenderItem.mBoardGridY = i;
            aRenderItem.mZPos = GetIceZPos(i);
            aRenderItemCount++;
        }
    }
    {
        int aZPos;
        if (mBoardData.mTimeStopCounter > 0) {
            aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
        } else if (mApp->mGameScene == GameScenes::SCENE_PLAYING || mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) {
            aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_BOTTOM, 0, 1);
        } else if (mCutScene->IsAfterSeedChooser() || mCutScene->IsInShovelTutorial() || mBoardData.mHelpIndex == AdviceType::ADVICE_CLICK_TO_CONTINUE) {
            aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_BOTTOM, 0, 1);
        } else {
            aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
        }

        AddUIRenderItem(
            aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_BACKDROP,
            MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_BOTTOM, 0, 0)
        );
        AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_BOTTOM_UI, aZPos);
        AddUIRenderItem(
            aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_COIN_BANK,
            MakeRenderOrder(RenderLayer::RENDER_LAYER_COIN_BANK, 0, 0)
        );
        AddUIRenderItem(
            aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_TOP_UI,
            MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_TOP, 0, 0)
        );
        AddUIRenderItem(
            aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_SCREEN_FADE,
            MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0)
        );
    }
    if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON) {
        int aZPos;
        if (StageHasRoof()) {
            aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, 0, 4);
        } else {
            aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, 3, 2);
        }
        AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_DOOR_MASK, aZPos);
    }
    if (StageHasFog()) {
        AddUIRenderItem(
            aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_FOG,
            MakeRenderOrder(RenderLayer::RENDER_LAYER_FOG, 0, 0)
        );
    }
    if (mApp->IsStormyNightLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS) {
        AddUIRenderItem(
            aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_STORM,
            MakeRenderOrder(RenderLayer::RENDER_LAYER_FOG, 0, 3)
        );
    }
    AddGameObjectRenderItemCursorPreview(
        aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_CURSOR_PREVIEW, mCursorPreview
    );

    TodHesitationTrace("start sort");
    std::sort(aRenderList, aRenderList + aRenderItemCount, RenderItemSortFunc);

    TodHesitationTrace("end sort, start draw");
    for (int i = 0; i < aRenderItemCount; i++) {
        const RenderItem &aRenderItem = aRenderList[i];
        switch (aRenderItem.mRenderObjectType) {
        case RenderObjectType::RENDER_ITEM_PLANT: {
            Plant *aPlant = aRenderItem.mPlant;
            if (aPlant->BeginDraw(g)) {
                aPlant->Draw(g);
                aPlant->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_PLANT_OVERLAY: {
            Plant *aPlant = aRenderItem.mPlant;
            if (aPlant->BeginDraw(g)) {
                mApp->mZenGarden->DrawPlantOverlay(g, aPlant);
                aPlant->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_PLANT_MAGNET_ITEMS: {
            Plant *aPlant = aRenderItem.mPlant;
            if (aPlant->BeginDraw(g)) {
                aPlant->DrawMagnetItems(g);
                aPlant->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_MOWER: {
            const LawnMower *aLawnMower = aRenderItem.mMower;
            aLawnMower->Draw(g);
            break;
        }

        case RenderObjectType::RENDER_ITEM_ZOMBIE: {
            Zombie *aZombie = aRenderItem.mZombie;
            if (aZombie->BeginDraw(g)) {
                aZombie->Draw(g);
                aZombie->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_ZOMBIE_SHADOW: {
            Zombie *aZombie = aRenderItem.mZombie;
            if (aZombie->BeginDraw(g)) {
                aZombie->DrawShadow(g);
                aZombie->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_ZOMBIE_BUNGEE_TARGET: {
            Zombie *aZombie = aRenderItem.mZombie;
            aZombie->DrawBungeeTarget(g);
            break;
        }

        case RenderObjectType::RENDER_ITEM_BOSS_PART: {
            Zombie *aBossZombie = GetBossZombie();
            if (aBossZombie && aBossZombie->BeginDraw(g)) {
                aBossZombie->DrawBossPart(g, aRenderItem.mBossPart);
                aBossZombie->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_COIN: {
            Coin *aCoin = aRenderItem.mCoin;
            if (aCoin->BeginDraw(g)) {
                aCoin->Draw(g);
                aCoin->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_PROJECTILE: {
            Projectile *aProjectile = aRenderItem.mProjectile;
            if (aProjectile->BeginDraw(g)) {
                aProjectile->Draw(g);
                aProjectile->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_PROJECTILE_SHADOW: {
            Projectile *aProjectile = aRenderItem.mProjectile;
            if (aProjectile->BeginDraw(g)) {
                aProjectile->DrawShadow(g);
                aProjectile->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_CURSOR_PREVIEW: {
            CursorPreview *aCursorPreview = aRenderItem.mCursorPreview;
            if (aCursorPreview->BeginDraw(g)) {
                aCursorPreview->Draw(g);
                aCursorPreview->EndDraw(g);
            }
            break;
        }

        case RenderObjectType::RENDER_ITEM_GRID_ITEM: {
            GridItem *aGridItem = aRenderItem.mGridItem;
            aGridItem->DrawGridItem(g);
            break;
        }

        case RenderObjectType::RENDER_ITEM_GRID_ITEM_OVERLAY: {
            const GridItem *aGridItem = aRenderItem.mGridItem;
            aGridItem->DrawGridItemOverlay(g);
            break;
        }

        case RenderObjectType::RENDER_ITEM_ICE: DrawIce(g, aRenderItem.mBoardGridY); break;

        case RenderObjectType::RENDER_ITEM_PARTICLE: {
            TodParticleSystem *aParticle = aRenderItem.mParticleSytem;
            aParticle->Draw(g);
            break;
        }

        case RenderObjectType::RENDER_ITEM_REANIMATION: {
            Reanimation *aReanimation = aRenderItem.mReanimation;
            aReanimation->Draw(g);
            break;
        }

        case RenderObjectType::RENDER_ITEM_COIN_BANK: DrawUICoinBank(g); break;

        case RenderObjectType::RENDER_ITEM_BACKDROP: DrawBackdrop(g); break;

        case RenderObjectType::RENDER_ITEM_DOOR_MASK: DrawHouseDoorTop(g); break;

        case RenderObjectType::RENDER_ITEM_BOTTOM_UI: DrawUIBottom(g); break;

        case RenderObjectType::RENDER_ITEM_TOP_UI: DrawUITop(g); break;

        case RenderObjectType::RENDER_ITEM_FOG: DrawFog(g); break;

        case RenderObjectType::RENDER_ITEM_STORM: mChallenge->DrawWeather(g); break;

        case RenderObjectType::RENDER_ITEM_SCREEN_FADE: DrawFadeOut(g); break;

        default: TOD_ASSERT(); break;
        }
    }

    TodHesitationTrace("end draw");
}

// 0x4173C0
bool Board::HasProgressMeter() {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST || mApp->IsFinalBossLevel() ||
        mApp->IsSlotMachineLevel() || mApp->IsSquirrelLevel() || mApp->IsIZombieLevel())
        return true;

    if (mBoardData.mProgressMeterWidth == 0) return false;

    if (mApp->IsContinuousChallenge() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || mApp->IsScaryPotterLevel())
        return false;

    return true;
}

// 0x417450
bool Board::ProgressMeterHasFlags() {
    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 1) return false;

    if (mApp->IsWhackAZombieLevel() || mApp->IsFinalBossLevel() ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST || mApp->IsSlotMachineLevel() ||
        mApp->IsSquirrelLevel() || mApp->IsIZombieLevel())
        return false;

    return true;
}

// 0x417500
//  GOTY @Patoke: 0x419E30
void Board::DrawProgressMeter(Graphics *g) {
    if (!HasProgressMeter()) return;

    // ====================================================================================================
    // ▲ 绘制进度条进度部分的贴图
    // ====================================================================================================
    g->DrawImageCel(Sexy::IMAGE_FLAGMETER, 600, 575, 0);
    const int aCelWidth = Sexy::IMAGE_FLAGMETER->GetCelWidth();
    const int aCelHeight = Sexy::IMAGE_FLAGMETER->GetCelHeight();
    const int aClipWidth =
        TodAnimateCurve(0, PROGRESS_METER_COUNTER, mBoardData.mProgressMeterWidth, 0, 143, TodCurves::CURVE_LINEAR);
    const Rect aSrcRect(aCelWidth - aClipWidth - 7, aCelHeight, aClipWidth, aCelHeight);
    const Rect aDstRect(aCelWidth - aClipWidth + 593, 575, aClipWidth, aCelHeight);
    g->DrawImage(Sexy::IMAGE_FLAGMETER, aDstRect, aSrcRect);

    // ====================================================================================================
    // ▲ 根据不同关卡，绘制进度条上的文字或旗帜
    // ====================================================================================================
    const int aPosX = aCelWidth / 2 + 600;
    const Color aColor(224, 187, 98);
    // @Patoke: updated these
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST) {
        const SexyString aMatchStr =
            fmt::format(_S("{}/{} {}"), mChallenge->mChallengeScore, 75, TodStringTranslate(_S("[MATCHES]")));
        TodDrawString(
            g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER
        );
    } else if (mApp->IsSquirrelLevel()) {
        const SexyString aMatchStr =
            fmt::format(_S("{}/{} {}"), mChallenge->mChallengeScore, 7, TodStringTranslate(_S("[SQUIRRELS]")));
        TodDrawString(
            g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER
        );
    } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_SLOT_MACHINE) {
        const int aSunMoney = ClampInt(mBoardData.mSunMoney, 0, 2000);
        const SexyString aMatchStr = fmt::format(_S("{}/{} {}"), aSunMoney, 2000, TodStringTranslate(_S("[SUN]")));
        TodDrawString(
            g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER
        );
    } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM) {
        const int aSunMoney = ClampInt(mBoardData.mSunMoney, 0, 1000);
        const SexyString aMatchStr = fmt::format(_S("{}/{} {}"), aSunMoney, 1000, TodStringTranslate(_S("[SUN]")));
        TodDrawString(
            g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER
        );
    } else if (mApp->IsIZombieLevel()) {
        const SexyString aMatchStr =
            fmt::format(_S("{}/{} {}"), mChallenge->mChallengeScore, 5, TodStringTranslate(_S("[BRAINS]")));
        TodDrawString(
            g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER
        );
    } else if (ProgressMeterHasFlags()) {
        const int aNumWavesPerFlag = GetNumWavesPerFlag();
        const int aNumFlagWaves = mBoardData.mNumWaves / aNumWavesPerFlag;
        const int aFlagsPosEnd = 590 + aCelWidth; // 旗帜区域的右界横坐标
        for (int aFlagWave = 1; aFlagWave <= aNumFlagWaves; aFlagWave++) {
            // 取得旗帜升起时的高度偏移
            int aHeight = 0;
            const int aTotalWavesAtFlag = aFlagWave * aNumWavesPerFlag;
            if (aTotalWavesAtFlag < mBoardData.mCurrentWave) {
                aHeight = 14;
            } else if (aTotalWavesAtFlag == mBoardData.mCurrentWave) {
                aHeight = TodAnimateCurve(100, 0, mBoardData.mFlagRaiseCounter, 0, 14, TodCurves::CURVE_LINEAR);
            }
            // 计算旗帜的横坐标
            const int aPosX =
                TodAnimateCurve(0, mBoardData.mNumWaves, aTotalWavesAtFlag, aFlagsPosEnd, 606, TodCurves::CURVE_LINEAR);
            // 绘制旗杆
            g->DrawImageCel(Sexy::IMAGE_FLAGMETERPARTS, aPosX, 571, 1, 0);
            // 绘制旗帜
            g->DrawImageCel(Sexy::IMAGE_FLAGMETERPARTS, aPosX, 572 - aHeight, 2, 0);
        }
    }

    // ====================================================================================================
    // ▲ 绘制进度条的额外部分
    // ====================================================================================================
    // 绘制“关卡进程”的小牌子
    g->DrawImage(Sexy::IMAGE_FLAGMETERLEVELPROGRESS, 638, 589);
    // 判断是否需要绘制进度条当前位置处的小僵尸头，不需要则直接返回
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || mApp->IsSquirrelLevel() ||
        mApp->IsSlotMachineLevel() || mApp->IsIZombieLevel() || mApp->IsFinalBossLevel())
        return;
    // 绘制僵尸头
    const int aHeadProgress = TodAnimateCurve(0, 150, mBoardData.mProgressMeterWidth, 0, 135, CURVE_LINEAR);
    g->DrawImageCel(Sexy::IMAGE_FLAGMETERPARTS, aCelWidth - aHeadProgress + 580, 572, 0, 0);
}

// 0x417C80
void Board::DrawHouseDoorBottom(Graphics *g) {
    switch (mBoardData.mBackground) {
    case BackgroundType::BACKGROUND_1_DAY:
        g->DrawImage(Sexy::IMAGE_BACKGROUND1_GAMEOVER_INTERIOR_OVERLAY, -126, 225);
        break;
    case BackgroundType::BACKGROUND_2_NIGHT:
        g->DrawImage(Sexy::IMAGE_BACKGROUND2_GAMEOVER_INTERIOR_OVERLAY, -125, 196);
        break;
    case BackgroundType::BACKGROUND_3_POOL:
        g->DrawImage(Sexy::IMAGE_BACKGROUND3_GAMEOVER_INTERIOR_OVERLAY, -171, 241);
        break;
    case BackgroundType::BACKGROUND_4_FOG:
        g->DrawImage(Sexy::IMAGE_BACKGROUND4_GAMEOVER_INTERIOR_OVERLAY, -172, 246);
        break;
    default: break;
    }
}

// 0x417CF0
void Board::DrawHouseDoorTop(Graphics *g) {
    switch (mBoardData.mBackground) {
    case BackgroundType::BACKGROUND_1_DAY:   g->DrawImage(Sexy::IMAGE_BACKGROUND1_GAMEOVER_MASK, -130, 202); break;
    case BackgroundType::BACKGROUND_2_NIGHT: g->DrawImage(Sexy::IMAGE_BACKGROUND2_GAMEOVER_MASK, -128, 207); break;
    case BackgroundType::BACKGROUND_3_POOL:  g->DrawImage(Sexy::IMAGE_BACKGROUND3_GAMEOVER_MASK, -172, 234); break;
    case BackgroundType::BACKGROUND_4_FOG:   g->DrawImage(Sexy::IMAGE_BACKGROUND4_GAMEOVER_MASK, -173, 133); break;
    case BackgroundType::BACKGROUND_5_ROOF:  g->DrawImage(Sexy::IMAGE_BACKGROUND5_GAMEOVER_MASK, -220, 81); break;
    case BackgroundType::BACKGROUND_6_BOSS:  g->DrawImage(Sexy::IMAGE_BACKGROUND6_GAMEOVER_MASK, -220, 81); break;
    default:                                 break;
    }
}

// 0x417DA0
//  GOTY @Patoke: 0x41A700
void Board::DrawLevel(Graphics *g) {
    // ====================================================================================================
    // ▲ 获取完整的关卡名称的字符串
    // ====================================================================================================
    SexyString aLevelStr;
    if (mApp->IsAdventureMode()) {
        aLevelStr = TodStringTranslate(_S("[LEVEL]")) + _S(" ") + mApp->GetStageString(mBoardData.mLevel);
    } else {
        aLevelStr = mApp->GetCurrentChallengeDef().mChallengeName;
        if (mApp->IsSurvivalMode() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
            const int aFlags = GetSurvivalFlagsCompleted();
            if (aFlags > 0) {
                const SexyString aFlagStr = mApp->Pluralize(aFlags, _S("[ONE_FLAG]"), _S("[COUNT_FLAGS]"));
                const SexyString aCompletedStr = TodReplaceString(_S("[FLAGS_COMPLETED]"), _S("{FLAGS}"), aFlagStr);
                aLevelStr = fmt::format(_S("{} - {}"), TodStringTranslate(aLevelStr), aCompletedStr);
            }
        } else if (mApp->IsEndlessIZombie(mApp->mGameMode) || mApp->IsEndlessScaryPotter(mApp->mGameMode)) {
            int aStreak = mChallenge->mSurvivalStage;
            if (mBoardData.mNextSurvivalStageCounter > 0) {
                aStreak++;
            }
            if (aStreak > 0) {
                const SexyString aStreakStr = TodReplaceNumberString(_S("[ENDLESS_STREAK]"), _S("{STREAK}"), aStreak);
                aLevelStr = fmt::format(_S("{} - {}"), TodStringTranslate(aLevelStr), aStreakStr);
            }
        }
    }

    // ====================================================================================================
    // ▲ 正式开始绘制关卡名称字符串
    // ====================================================================================================
    int aPosX = 780;
    int aPosY = 595;
    if (HasProgressMeter()) {
        aPosX = 593;
    }
    if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING) {
        aPosY += TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, 0, 50, TodCurves::CURVE_EASE_IN_OUT);
    }
    TodDrawString(
        g, aLevelStr, aPosX, aPosY, Sexy::FONT_HOUSEOFTERROR16, Color(224, 187, 98),
        DrawStringJustification::DS_ALIGN_RIGHT
    );
}

// 0x4182D0
void Board::DrawZenWheelBarrowButton(Graphics *g, const int theOffsetY) {
    Rect aButtonRect = GetShovelButtonRect();
    GetZenButtonRect(GameObjectType::OBJECT_TYPE_WHEELBARROW, aButtonRect);
    PottedPlant *aPlant = mApp->mZenGarden->GetPottedPlantInWheelbarrow();
    if (aPlant && mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW) {
        if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING) {
            g->DrawImage(Sexy::IMAGE_ZEN_WHEELBARROW, aButtonRect.mX - 7, aButtonRect.mY + theOffsetY - 3);
        } else {
            g->DrawImage(Sexy::IMAGE_ZEN_WHEELBARROW, aButtonRect.mX - 7, aButtonRect.mY + theOffsetY + 4);
        }

        if (aPlant->mPlantAge == PottedPlantAge::PLANTAGE_SMALL) {
            mApp->mZenGarden->DrawPottedPlant(
                g, aButtonRect.mX + 23, aButtonRect.mY + theOffsetY - 8, aPlant, 0.6f, true
            );
        } else if (aPlant->mPlantAge == PottedPlantAge::PLANTAGE_MEDIUM) {
            mApp->mZenGarden->DrawPottedPlant(
                g, aButtonRect.mX + 28, aButtonRect.mY + theOffsetY + 2, aPlant, 0.5f, true
            );
        } else {
            mApp->mZenGarden->DrawPottedPlant(
                g, aButtonRect.mX + 34, aButtonRect.mY + theOffsetY + 12, aPlant, 0.4f, true
            );
        }
    } else {
        g->DrawImage(Sexy::IMAGE_ZEN_WHEELBARROW, aButtonRect.mX - 7, aButtonRect.mY + theOffsetY - 3);
    }
}

// 0x418500
void Board::DrawZenButtons(Graphics *g) {
    int aOffsetY = 0;
    if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING) {
        aOffsetY = TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, 0, -72, TodCurves::CURVE_EASE_IN_OUT);
    }

    for (GameObjectType aTool = GameObjectType::OBJECT_TYPE_WATERING_CAN;
         aTool <= GameObjectType::OBJECT_TYPE_NEXT_GARDEN; aTool = static_cast<GameObjectType>(aTool + 1)) {
        if (!CanUseGameObject(aTool)) continue;

        Rect aButtonRect = GetShovelButtonRect();
        if (aTool == GameObjectType::OBJECT_TYPE_NEXT_GARDEN) {
            aButtonRect.mX = 564;
            if (!mMenuButton->mBtnNoDraw) {
                g->DrawImage(Sexy::IMAGE_ZEN_NEXTGARDEN, aButtonRect.mX + 2, aButtonRect.mY + aOffsetY);
            }
        } else {
            GetZenButtonRect(aTool, aButtonRect);
            g->DrawImage(Sexy::IMAGE_SHOVELBANK, aButtonRect.mX, aButtonRect.mY + aOffsetY);
            if (static_cast<int>(mCursorObject->mCursorType) ==
                static_cast<int>(CursorType::CURSOR_TYPE_WATERING_CAN) + static_cast<int>(aTool) - 6) {
                continue; // 如果工具正在被手持，则跳过绘制
            }

            if (aTool == GameObjectType::OBJECT_TYPE_WATERING_CAN) {
                if (mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_GOLD_WATERINGCAN)]) {
                    g->DrawImage(Sexy::IMAGE_WATERINGCANGOLD, aButtonRect.mX - 2, aButtonRect.mY + aOffsetY - 6);
                } else {
                    g->DrawImage(Sexy::IMAGE_WATERINGCAN, aButtonRect.mX - 2, aButtonRect.mY + aOffsetY - 6);
                }
            } else if (aTool == GameObjectType::OBJECT_TYPE_FERTILIZER) {
                const int aCharges = mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_FERTILIZER);
                if (aCharges == 0) {
                    g->SetColorizeImages(true);
                    g->SetColor(Color(96, 96, 96));
                } else if (mBoardData.mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_FERTILIZE_PLANTS) {
                    g->SetColorizeImages(true);
                    g->SetColor(GetFlashingColor(mBoardData.mMainCounter, 75));
                }
                g->DrawImage(Sexy::IMAGE_FERTILIZER, aButtonRect.mX - 6, aButtonRect.mY + aOffsetY - 7);
                g->SetColorizeImages(false);

                SexyString aChargeString = fmt::format(_S("x{}"), aCharges);
                TodDrawString(
                    g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16,
                    Color::White, DrawStringJustification::DS_ALIGN_RIGHT
                );
            } else if (aTool == GameObjectType::OBJECT_TYPE_BUG_SPRAY) {
                const int aCharges = mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_BUG_SPRAY);
                if (aCharges == 0) {
                    g->SetColorizeImages(true);
                    g->SetColor(Color(128, 128, 128));
                }
                g->DrawImage(
                    Sexy::IMAGE_REANIM_ZENGARDEN_BUGSPRAY_BOTTLE, aButtonRect.mX, aButtonRect.mY + aOffsetY - 1
                );
                g->SetColorizeImages(false);

                SexyString aChargeString = fmt::format(_S("x{}"), aCharges);
                TodDrawString(
                    g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16,
                    Color::White, DrawStringJustification::DS_ALIGN_RIGHT
                );
            } else if (aTool == GameObjectType::OBJECT_TYPE_PHONOGRAPH) {
                g->DrawImage(Sexy::IMAGE_PHONOGRAPH, aButtonRect.mX + 2, aButtonRect.mY + aOffsetY + 2);
            } else if (aTool == GameObjectType::OBJECT_TYPE_CHOCOLATE) {
                const int aCharges = mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_CHOCOLATE);
                if (aCharges == 0) {
                    g->SetColorizeImages(true);
                    g->SetColor(Color(128, 128, 128));
                }
                g->DrawImage(Sexy::IMAGE_CHOCOLATE, aButtonRect.mX + 6, aButtonRect.mY + aOffsetY + 4);
                g->SetColorizeImages(false);

                SexyString aChargeString = fmt::format(_S("x{}"), aCharges);
                TodDrawString(
                    g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16,
                    Color::White, DrawStringJustification::DS_ALIGN_RIGHT
                );
            } else if (aTool == GameObjectType::OBJECT_TYPE_GLOVE) {
                if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE &&
                    mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW) {
                    g->DrawImage(Sexy::IMAGE_ZEN_GARDENGLOVE, aButtonRect.mX - 6, aButtonRect.mY + aOffsetY - 4);
                }
            } else if (aTool == GameObjectType::OBJECT_TYPE_MONEY_SIGN) {
                g->DrawImage(Sexy::IMAGE_ZEN_MONEYSIGN, aButtonRect.mX - 5, aButtonRect.mY + aOffsetY - 4);
            } else if (aTool == GameObjectType::OBJECT_TYPE_WHEELBARROW) {
                DrawZenWheelBarrowButton(g, aOffsetY);
            } else if (aTool == GameObjectType::OBJECT_TYPE_TREE_FOOD) {
                int aCharges = mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_TREE_FOOD);
                if (aCharges <= 0) {
                    g->SetColorizeImages(true);
                    g->SetColor(Color(128, 128, 128));
                    aCharges = 0;
                }
                if (!mChallenge->TreeOfWisdomCanFeed()) {
                    g->SetColorizeImages(true);
                    g->SetColor(Color(128, 128, 128));
                }
                g->DrawImage(Sexy::IMAGE_TREEFOOD, aButtonRect.mX - 6, aButtonRect.mY + aOffsetY - 7);
                g->SetColorizeImages(false);

                SexyString aChargeString = fmt::format(_S("x{}"), aCharges);
                TodDrawString(
                    g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16,
                    Color::White, DrawStringJustification::DS_ALIGN_RIGHT
                );
            }
        }
    }
}

// 0x418B70
void Board::DrawShovel(Graphics *g) {
    if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
        mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM) {
        if (mBoardData.mShowShovel) {
            const Rect aShovelRect = GetShovelButtonRect();
            g->DrawImage(Sexy::IMAGE_SHOVELBANK, aShovelRect.mX, aShovelRect.mY);

            if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_SHOVEL) {
                if (mChallenge->mChallengeState == static_cast<ChallengeState>(15)) {
                    g->SetColorizeImages(true);
                    g->SetColor(GetFlashingColor(mBoardData.mMainCounter, 75));
                }
                g->DrawImage(Sexy::IMAGE_SHOVEL, aShovelRect.mX - 7, aShovelRect.mY - 3);
                g->SetColorizeImages(false);
            }
        }
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        DrawZenButtons(g);
    }
}

// 0x418C70
void Board::DrawDebugText(Graphics *g) {
    SexyString aText;

    switch (mBoardData.mDebugTextMode) {
    case DebugTextMode::DEBUG_TEXT_NONE: break;

    case DebugTextMode::DEBUG_TEXT_ZOMBIE_SPAWN: {
        int aTime = mBoardData.mZombieCountDownStart - mBoardData.mZombieCountDown;
        float aCountDownFraction = static_cast<float>(aTime) / static_cast<float>(mBoardData.mZombieCountDownStart);

        aText += _S("ZOMBIE SPAWNING DEBUG\n");
        aText += fmt::format(_S("CurrentWave: {} of {}\n"), mBoardData.mCurrentWave, mBoardData.mNumWaves);
        aText += fmt::format(_S("TimeSinseLastSpawn: {} {}\n"), aTime, aTime > 400 ? _S("") : _S("(too soon)"));
        aText += fmt::format(
            _S("ZombieCountDown: {}/{} ({:.0f}%)\n"), mBoardData.mZombieCountDown, mBoardData.mZombieCountDownStart,
            aCountDownFraction
        );

        if (mBoardData.mZombieHealthToNextWave != -1) {
            int aTotalHealth = TotalZombiesHealthInWave(mBoardData.mCurrentWave - 1);
            int aHealthRange = std::max(mBoardData.mZombieHealthWaveStart - mBoardData.mZombieHealthToNextWave, 1);
            float aHealthFraction =
                static_cast<float>(mBoardData.mZombieHealthToNextWave - aTotalHealth + aHealthRange) /
                static_cast<float>(aHealthRange);
            aText += fmt::format(
                _S("ZombieHealth: CurZombieHealth {} trigger {} ({:.0f}%)\n"), aTotalHealth,
                mBoardData.mZombieHealthToNextWave, aHealthFraction * 100
            );
        } else {
            aText += _S("ZombieHealth: before first wave\n");
        }

        if (mBoardData.mHugeWaveCountDown > 0) {
            aText += fmt::format(_S("HugeWaveCountDown: {}\n"), mBoardData.mHugeWaveCountDown);
        }

        Zombie *aBossZombie = GetBossZombie();
        if (aBossZombie) {
            aText += fmt::format(_S("\nSpawn: {}\n"), aBossZombie->mSummonCounter);
            aText += fmt::format(_S("Stomp: {}\n"), aBossZombie->mBossStompCounter);
            aText += fmt::format(_S("Bungee: {}\n"), aBossZombie->mBossBungeeCounter);
            aText += fmt::format(_S("Head: {}\n"), aBossZombie->mBossHeadCounter);
            aText += fmt::format(_S("Health: {} of {}\n"), aBossZombie->mBodyHealth, aBossZombie->mBodyMaxHealth);
        }

        break;
    }

    case DebugTextMode::DEBUG_TEXT_MUSIC: {
        aText += _S("MUSIC DEBUG\n");
        aText += fmt::format(_S("CurrentWave: {} of {}\n"), mBoardData.mCurrentWave, mBoardData.mNumWaves);

        if (mApp->mMusic->mCurMusicFileMain == MusicFile::MUSIC_FILE_NONE) {
            aText += _S("No music");
        } else {
            aText += _S("Music Burst: ");

            if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_OFF) {
                aText += _S("Off");
            } else if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_STARTING) {
                aText += fmt::format(_S("Starting {}/{}"), mApp->mMusic->mBurstStateCounter, 400);
            } else if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_ON) {
                aText += fmt::format(_S("On at least until {}/{}"), mApp->mMusic->mBurstStateCounter, 800);
            } else if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_FINISHING) {
                aText += fmt::format(_S("Finishing {}/{}"), mApp->mMusic->mBurstStateCounter, 400);
            }

            if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF) {
                aText += _S(", Drums off");
            } else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_ON_QUEUED) {
                aText += _S(", Drums queued on");
            } else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_ON) {
                aText += _S(", Drums on");
            } else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF_QUEUED) {
                aText += _S(", Drums queued off");
            } else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_FADING) {
                aText += fmt::format(_S(", Drums fading on {}/{}"), mApp->mMusic->mDrumsStateCounter, 50);
            }
            aText += _S("\n");

            int aPackedOrderMain = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileMain);
            int aCurrentOrder = LOWORD(aPackedOrderMain);
            aText += fmt::format(
                _S("Music order {:02d} row {:02d}\n"), LOWORD(aPackedOrderMain), HIWORD(aPackedOrderMain) / 4
            );
            if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK ||
                mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES ||
                mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST ||
                mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF) {
                int aPackedOrderHihats = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileHihats);
                int aPackedOrderDrums = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileDrums);
                if (aCurrentOrder == LOWORD(aPackedOrderHihats) && aCurrentOrder == LOWORD(aPackedOrderDrums)) {
                    int aDiffHihats = HIWORD(aPackedOrderHihats) - HIWORD(aPackedOrderMain);
                    int aDiffDrums = HIWORD(aPackedOrderDrums) - HIWORD(aPackedOrderMain);
                    if (abs(aDiffHihats) > 1 || abs(aDiffDrums) > 1) {
                        aText += fmt::format(_S("Music unsynced hihats {} drums {}\n"), aDiffHihats, aDiffDrums);
                    }
                }

                HMUSIC aMusicHandle1 = mApp->mMusic->GetBassMusicHandle(mApp->mMusic->mCurMusicFileMain);
                HMUSIC aMusicHandle2 = mApp->mMusic->GetBassMusicHandle(mApp->mMusic->mCurMusicFileHihats);
                HMUSIC aMusicHandle3 = mApp->mMusic->GetBassMusicHandle(mApp->mMusic->mCurMusicFileDrums);
                float bpm1;
                float bpm2;
                float bpm3;
                BASS_ChannelGetAttribute(aMusicHandle1, BASS_ATTRIB_MUSIC_BPM, &bpm1);
                BASS_ChannelGetAttribute(aMusicHandle2, BASS_ATTRIB_MUSIC_BPM, &bpm2);
                BASS_ChannelGetAttribute(aMusicHandle3, BASS_ATTRIB_MUSIC_BPM, &bpm3);
                aText += fmt::format(_S("bpm1 {} bmp2 {} bpm3 {}\n"), bpm1, bpm2, bpm3);
            } else if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS) {
                int aPackedOrderDrums = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileDrums);
                aText += fmt::format(
                    _S("Drum order %02d row %02d\n"), LOWORD(aPackedOrderDrums), HIWORD(aPackedOrderDrums) / 4
                );
                int aDiffDrums = HIWORD(aPackedOrderDrums) - HIWORD(aPackedOrderMain);
                if (abs(aDiffDrums) > 0 && abs(aDiffDrums) <= 128) {
                    aText += fmt::format(_S("Drums unsynced %d\n"), aDiffDrums);
                }
            }
        }

        break;
    }

    case DebugTextMode::DEBUG_TEXT_MEMORY:
        aText += _S("MEMORY DEBUG\n");
        aText += fmt::format(_S("attachments {}\n"), mApp->mEffectSystem->mAttachmentHolder->mAttachments.mSize);
        aText += fmt::format(_S("emitters {}\n"), mApp->mEffectSystem->mParticleHolder->mEmitters.mSize);
        aText += fmt::format(_S("particles {}\n"), mApp->mEffectSystem->mParticleHolder->mParticles.mSize);
        aText += fmt::format(_S("particle systems {}\n"), mApp->mEffectSystem->mParticleHolder->mParticleSystems.mSize);
        aText += fmt::format(_S("trails {}\n"), mApp->mEffectSystem->mTrailHolder->mTrails.mSize);
        aText += fmt::format(_S("reanimation {}\n"), mApp->mEffectSystem->mReanimationHolder->mReanimations.mSize);
        aText += fmt::format(_S("zombies {}\n"), mZombies.mSize);
        aText += fmt::format(_S("plants {}\n"), mPlants.mSize);
        aText += fmt::format(_S("projectiles {}\n"), mProjectiles.mSize);
        aText += fmt::format(_S("coins {}\n"), mCoins.mSize);
        aText += fmt::format(_S("lawn mowers {}\n"), mLawnMowers.mSize);
        aText += fmt::format(_S("grid items {}\n"), mGridItems.mSize);
        break;

    case DebugTextMode::DEBUG_TEXT_COLLISION: aText += _S("COLLISION DEBUG\n"); break;

    default: TOD_ASSERT(); break;
    }

    // unreachable();
    //  TODO
    mDebugFont = FONT_BRIANNETOD16;
    g->SetFont(mDebugFont);
    g->SetColor(Color::Black);
    g->DrawStringWordWrapped(aText, 10, 89);
    g->DrawStringWordWrapped(aText, 11, 91);
    g->DrawStringWordWrapped(aText, 9, 90);
    g->DrawStringWordWrapped(aText, 11, 90);
    g->SetColor(Color(255, 255, 255));
    g->DrawStringWordWrapped(aText, 10, 90);
}

// 0x419AE0
void Board::DrawDebugObjectRects(Graphics *g) {
    if (mBoardData.mDebugTextMode != DebugTextMode::DEBUG_TEXT_COLLISION) return;

    {
        Plant *aPlant = nullptr;
        while (IteratePlants(aPlant)) {
            Rect aRect = aPlant->GetPlantRect();
            g->SetColor(Color(0, 255, 0));
            g->DrawRect(aRect);

            Rect aAttackRect = aPlant->GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);
            if (aAttackRect.mWidth < BOARD_WIDTH) {
                g->SetColor(Color(255, 0, 0));
                g->DrawRect(aAttackRect);
            }

            Rect aSecondaryRect = aPlant->GetPlantAttackRect(PlantWeapon::WEAPON_SECONDARY);
            if (aSecondaryRect.mWidth < BOARD_WIDTH) {
                g->SetColor(Color(255, 0, 128));
                g->DrawRect(aSecondaryRect);
            }
        }
    }
    {
        Zombie *aZombie = nullptr;
        while (IterateZombies(aZombie)) {
            if (!aZombie->IsDeadOrDying()) {
                Rect aRect = aZombie->GetZombieRect();
                g->SetColor(Color(0, 255, 0));
                g->DrawRect(aRect);

                Rect aAttackRect = aZombie->GetZombieAttackRect();
                g->SetColor(Color(255, 0, 0));
                g->DrawRect(aAttackRect);
            }
        }
    }
    {
        LawnMower *aLawnMower = nullptr;
        while (IterateLawnMowers(aLawnMower)) {
            Rect aAttackRect = aLawnMower->GetLawnMowerAttackRect();
            g->SetColor(Color(255, 0, 0));
            g->DrawRect(aAttackRect);
        }
    }
    {
        Projectile *aProjectile = nullptr;
        while (IterateProjectiles(aProjectile)) {
            g->SetColor(Color(255, 0, 0));
            Rect aDamageRect = aProjectile->GetProjectileRect();
            g->DrawRect(aDamageRect);
        }
    }
}

// 0x419EB0
void Board::DrawFadeOut(Graphics *g) {
    if (mBoardData.mBoardFadeOutCounter < 0 || IsSurvivalStageWithRepick()) return;

    const int anAlpha = TodAnimateCurve(200, 0, mBoardData.mBoardFadeOutCounter, 0, 255, TodCurves::CURVE_LINEAR);
    if (mBoardData.mLevel == 9 || mBoardData.mLevel == 19 || mBoardData.mLevel == 29 || mBoardData.mLevel == 39 ||
        mBoardData.mLevel == 49) {
        g->SetColor(Color(0, 0, 0, anAlpha));
    } else {
        g->SetColor(Color(255, 255, 255, anAlpha));
    }
    g->FillRect(0, 0, mWidth, mHeight);
}

// 0x419F60
void Board::DrawTopRightUI(Graphics *g) {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING) {
            mMenuButton->mY =
                TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, -10, -50, TodCurves::CURVE_EASE_IN_OUT);
            mStoreButton->mX =
                TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, 678, 800, TodCurves::CURVE_EASE_IN_OUT);
        } else {
            mMenuButton->mY = -10;
            mStoreButton->mX = 678;
        }
    }

    if (mBoardData.mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_COMPLETED) {
        g->SetColorizeImages(true);
        g->SetColor(GetFlashingColor(mBoardData.mMainCounter, 75));
    }
    mMenuButton->Draw(g);
    g->SetColorizeImages(false);

    if (mStoreButton && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
        if (mBoardData.mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_VISIT_STORE) {
            g->SetColorizeImages(true);
            g->SetColor(GetFlashingColor(mBoardData.mMainCounter, 75));
        }
        mStoreButton->Draw(g);
        g->SetColorizeImages(false);
    }
}

// 0x41A0A0
void Board::DrawUIBottom(Graphics *g) {
    if (mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
        const int aWaveTime = abs(mBoardData.mMainCounter / 8 % 22 - 11);
        g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
        g->DrawImageCel(Sexy::IMAGE_WAVESIDE, 0, 40, aWaveTime);
        g->DrawImageCel(Sexy::IMAGE_WAVECENTER, 160, 40, aWaveTime);
        g->DrawImageCel(Sexy::IMAGE_WAVECENTER, 320, 40, aWaveTime);
        g->DrawImageCel(Sexy::IMAGE_WAVECENTER, 480, 40, aWaveTime);

        TodDrawImageCelScaledF(
            g, Sexy::IMAGE_WAVESIDE, 800, 40, aWaveTime % Sexy::IMAGE_WAVESIDE->mNumCols,
            aWaveTime / Sexy::IMAGE_WAVESIDE->mNumCols, -1.0f, 1.0f
        );
        // TodDrawImageCelScaled(g, Sexy::IMAGE_WAVESIDE, 800, 40, 0, aWaveTime, -1.0f, 1.0f);
        g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
    }

    if (mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
        mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
        g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
        g->DrawImage(
            IMAGE_BACKGROUND_GREENHOUSE_OVERLAY, Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT),
            Rect(0, 0, IMAGE_BACKGROUND_GREENHOUSE_OVERLAY->mWidth, IMAGE_BACKGROUND_GREENHOUSE_OVERLAY->mHeight)
        );
        g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
    }

    if (mApp->mGameScene != GameScenes::SCENE_ZOMBIES_WON) {
        if (mSeedBank->BeginDraw(g)) {
            mSeedBank->Draw(g);
            mSeedBank->EndDraw(g);
        }

        if (mAdvice->mMessageStyle == MessageStyle::MESSAGE_STYLE_SLOT_MACHINE) {
            mAdvice->Draw(g);
        }
    }

    DrawShovel(g);
    if (!StageHasFog()) {
        DrawTopRightUI(g);
    }
}

// 0x41A2A0
void Board::DrawUICoinBank(Graphics *g) {
    if (mApp->mGameScene != GameScenes::SCENE_PLAYING && mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_OFF)
        return;

    if (mBoardData.mCoinBankFadeCount <= 0) return;

    int aPosX = 57;
    const int aPosY = 599 - Sexy::IMAGE_COINBANK->GetHeight();
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_OFF) {
        aPosX = 450 - mX;
    }

    g->SetColorizeImages(true);
    const int anAlpha = ClampInt(255 * mBoardData.mCoinBankFadeCount / 15, 0, 255);
    g->SetColor(Color(255, 255, 255, anAlpha));
    g->DrawImage(Sexy::IMAGE_COINBANK, aPosX, aPosY);

    g->SetColor(Color(180, 255, 90, anAlpha));
    g->SetFont(Sexy::FONT_CONTINUUMBOLD14);
    const SexyString aCoinLabel = mApp->GetMoneyString(mApp->mPlayerInfo->mCoins);
    g->DrawString(aCoinLabel, aPosX + 116 - Sexy::FONT_CONTINUUMBOLD14->StringWidth(aCoinLabel), aPosY + 24);
    g->SetColorizeImages(false);
}

// 0x41A430
void Board::ClearFogAroundPlant(const Plant *thePlant, const int theSize) {
    // int aFogFadeOutSpeed = mFogBlownCountDown >= 2000 ? 40 : mFogBlownCountDown > 0 ? 2 : 6;
    int aFogFadeOutSpeed = 6;
    if (mBoardData.mFogBlownCountDown > 0 && mBoardData.mFogBlownCountDown < 2000) {
        aFogFadeOutSpeed = 2;
    } else if (mBoardData.mFogBlownCountDown > 0) {
        aFogFadeOutSpeed = 40;
    }

    const int aLeft = LeftFogColumn();
    const int aFogOffsetX = (mBoardData.mFogOffset + 50) / 100;
    int aStartX = thePlant->mPlantCol - theSize - aFogOffsetX;
    int aEndX = thePlant->mPlantCol + theSize - aFogOffsetX;
    aStartX = std::max(aStartX, aLeft);
    aEndX = std::min(aEndX, MAX_GRID_SIZE_X - 1);

    int aStartY = thePlant->mRow - theSize;
    int aEndY = thePlant->mRow + theSize;
    aStartY = std::max(aStartY, 0);
    aEndY = std::min(aEndY, MAX_GRID_SIZE_Y);

    for (int x = aStartX; x <= aEndX; x++) {
        for (int y = aStartY; y <= aEndY; y++) {
            const int aDistX = abs(x + aFogOffsetX - thePlant->mPlantCol);
            const int aDistY = abs(y - thePlant->mRow);
            if (theSize == 4) {
                if (aDistX > 3 || aDistY > 2) {
                    continue;
                }
                if (aDistX + aDistY == 5) {
                    continue;
                }
            } else if (aDistX + aDistY > theSize) {
                continue;
            }

            mBoardData.mGridCelFog[x][y] = std::max(mBoardData.mGridCelFog[x][y] - aFogFadeOutSpeed, 0);
        }
    }
}

// 0x41A5D0
void Board::UpdateFog() {
    if (!StageHasFog()) return;

    // int aFogFadeInSpeed = mFogBlownCountDown >= 2000 ? 20 : mFogBlownCountDown > 0 ? 1 : 3;
    int aFogFadeInSpeed = 3;
    if (mBoardData.mFogBlownCountDown > 0 && mBoardData.mFogBlownCountDown < 2000) {
        aFogFadeInSpeed = 1;
    } else if (mBoardData.mFogBlownCountDown > 0) {
        aFogFadeInSpeed = 20;
    }

    const int aLeft = LeftFogColumn();
    for (int x = aLeft; x < MAX_GRID_SIZE_X; x++) {
        for (int y = 0; y < MAX_GRID_SIZE_Y + 1; y++) {
            int aFogMax = x == aLeft ? 200 : 255;
            mBoardData.mGridCelFog[x][y] = std::min(mBoardData.mGridCelFog[x][y] + aFogFadeInSpeed, aFogMax);
        }
    }

    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->NotOnGround()) continue;

        if (aPlant->mSeedType == SeedType::SEED_PLANTERN) {
            ClearFogAroundPlant(aPlant, 4);
        } else if (aPlant->mSeedType == SeedType::SEED_TORCHWOOD) {
            ClearFogAroundPlant(aPlant, 1);
        }
    }
}

// 0x41A730
void Board::DrawFog(Graphics *g) {
    Image *aImageFog = /*mApp->Is3DAccelerated() ?*/ Sexy::IMAGE_FOG;
    /*: Sexy::IMAGE_FOG_SOFTWARE*/
    for (int x = 0; x < MAX_GRID_SIZE_X; x++) {
        for (int y = 0; y < MAX_GRID_SIZE_Y + 1; y++) {
            const int aFadeAmount = mBoardData.mGridCelFog[x][y];
            if (aFadeAmount == 0) continue;

            // 取得格子内的雾的形状（第 6 行的雾的形状采用与第 0 行相同）
            // { sub eax,edx } 向前 [y / 6] 列，但 y 超出上限 y - 5 行，故相当于列不变，行 = y % 6；
            const int aCelLook = mBoardData.mGridCelLook[x][y % MAX_GRID_SIZE_Y];
            const int aCelCol = aCelLook % 8;
            // 本格浓雾横坐标 = 列 * 80 + 浓雾偏移 - 15，纵坐标 = 行 * 85 + 20
            const float aPosX = x * 80 + mBoardData.mFogOffset - 15;
            const float aPosY = y * 85 + 20;
            // 开始计算周期变化的颜色，aTime 为根据主计时计算的时间
            const float aTime = mBoardData.mMainCounter * PI * 2;
            // 与行、列有关的初始相位
            const float aPhaseX = 6 * PI * x / MAX_GRID_SIZE_X;
            const float aPhaseY = 6 * PI * y / (MAX_GRID_SIZE_Y + 1);
            // 根据初相和时间计算当前相位
            const float aMotion = 13 + 4 * sin(aTime / 900 + aPhaseY) + 8 * sin(aTime / 500 + aPhaseX);

            const int aColorVariant = 255 - aCelLook * 1.5 - aMotion * 1.5;
            const int aLightnessVariant = 255 - aCelLook - aMotion;
            /*
            if (!mApp->Is3DAccelerated())
            {
                aPosX += 10;
                aPosY += 3;
                aCelCol = aCelLook % Sexy::IMAGE_FOG_SOFTWARE->mNumCols;
                aColorVariant = 255;
                aLightnessVariant = 255;
            }*/

            g->SetColorizeImages(true);
            g->SetColor(Color(aColorVariant, aColorVariant, aLightnessVariant, aFadeAmount));
            g->DrawImageCel(aImageFog, aPosX, aPosY, aCelCol, 0);

            if (x == MAX_GRID_SIZE_X - 1) {
                g->DrawImageCel(aImageFog, aPosX + 80, aPosY, aCelCol, 0);
            }
            g->SetColorizeImages(false);
        }
    }
}

// 0x41AA00
bool Board::IsScaryPotterDaveTalking() {
    return mApp->IsScaryPotterLevel() && mBoardData.mNextSurvivalStageCounter > 0 &&
           mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_OFF;
}

// 0x41AA40
void Board::DrawUITop(Graphics *g) {
    if (StageHasFog()) {
        DrawTopRightUI(g);
    }

    if (mBoardData.mTimeStopCounter > 0) {
        g->SetColor(Color(200, 200, 200, 210));
        g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
    }

    if (mApp->mGameScene == GameScenes::SCENE_PLAYING || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        DrawProgressMeter(g);
        DrawLevel(g);
    }
    if (mStoreButton && mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND) {
        mStoreButton->Draw(g);
    }

    if ((mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mGameMode == GameMode::GAMEMODE_INTRO) &&
        mCutScene->mUpsellHideBoard) {
        g->SetColor(Color(0, 0, 0));
        g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL) {
        mCutScene->DrawUpsell(g);
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_INTRO) {
        mCutScene->DrawIntro(g);
    }

    if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || IsScaryPotterDaveTalking()) {
        Graphics aScreenSpace(*g);
        aScreenSpace.mTransX -= mX;
        aScreenSpace.mTransY -= mY;
        mApp->DrawCrazyDave(&aScreenSpace);
    }

    if (mAdvice->mMessageStyle != MessageStyle::MESSAGE_STYLE_SLOT_MACHINE) {
        mAdvice->Draw(g);
    }

    if (mBoardData.mTimeStopCounter == 0 && mCursorObject->BeginDraw(g)) {
        mCursorObject->Draw(g);
        mCursorObject->EndDraw(g);
    }

    mToolTip->Draw(g);
    DrawDebugText(g);
    DrawDebugObjectRects(g);
}

// 0x41ACF0
void Board::Draw(Graphics *g) {
    if (mApp->GetDialog(Dialogs::DIALOG_STORE) || mApp->GetDialog(Dialogs::DIALOG_ALMANAC)) return;

    g->SetLinearBlend(true);

    if (mBoardData.mDrawCount && mCutScene->mPreloaded) {
        const clock_t aTickCount = clock();
        const clock_t aIntervalDraws = mBoardData.mDrawCount - mBoardData.mIntervalDrawCountStart;
        const clock_t aInterval = aTickCount - mBoardData.mIntervalDrawTime;
        if (aInterval > 10 * CLOCKS_PER_SEC) {
            const float aIntervalFPS = ((1000.0 * 1000.0 / CLOCKS_PER_SEC) * aIntervalDraws + 500) / aInterval;
            if (mBoardData.mMinFPS > aIntervalFPS) {
                mBoardData.mMinFPS = aIntervalFPS;
            }
            mBoardData.mIntervalDrawCountStart = mBoardData.mDrawCount;
            mBoardData.mIntervalDrawTime = aTickCount;
        }
    } else {
        ResetFPSStats();
    }

    mBoardData.mDrawCount++;
    DrawGameObjects(g);
}

// 0x41AE60
//  GOTY @Patoke: 0x41D910
void Board::SetMustacheMode(const bool theEnableMustache) {
    mApp->PlayFoley(FoleyType::FOLEY_POLEVAULT);
    mBoardData.mMustacheMode = theEnableMustache;
    mApp->mMustacheMode = theEnableMustache;

    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        aZombie->EnableMustache(theEnableMustache);
    }
}

// 0x41AF00
void Board::SetFutureMode(const bool theEnableFuture) {
    mApp->PlaySample(Sexy::SOUND_BOING);
    mBoardData.mFutureMode = theEnableFuture;
    mApp->mFutureMode = theEnableFuture;

    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        aZombie->EnableFuture(theEnableFuture);
    }
}

// 0x41AF60
void Board::SetPinataMode(const bool theEnablePinata) {
    mApp->PlayFoley(FoleyType::FOLEY_JUICY);
    mBoardData.mPinataMode = theEnablePinata;
    mApp->mPinataMode = theEnablePinata;
}

// 0x41AFD0
void Board::SetDanceMode(const bool theEnableDance) {
    mApp->PlayFoley(FoleyType::FOLEY_DANCER);
    mBoardData.mDanceMode = theEnableDance;
    mApp->mDanceMode = theEnableDance;

    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (!aZombie->mDead) {
            aZombie->EnableDance();
        }
    }
}

// 0x41B0D0
void Board::SetSuperMowerMode(const bool theEnableSuperMower) {
    mApp->PlayFoley(FoleyType::FOLEY_ZAMBONI);
    mBoardData.mSuperMowerMode = theEnableSuperMower;
    mApp->mSuperMowerMode = theEnableSuperMower;

    LawnMower *aLawnMower = nullptr;
    while (IterateLawnMowers(aLawnMower)) {
        aLawnMower->EnableSuperMower(theEnableSuperMower);
    }
}

void Board::SetDaisyMode(const bool theEnableDaisy) {
    mApp->PlaySample(SOUND_LOADINGBAR_FLOWER);
    mBoardData.mDaisyMode = theEnableDaisy;
    mApp->mDaisyMode = theEnableDaisy;
}

void Board::SetSukhbirMode(const bool theEnableSukhbir) {
    mApp->PlaySample(Sexy::SOUND_SUKHBIR);
    mBoardData.mSukhbirMode = theEnableSukhbir;
    mApp->mSukhbirMode = theEnableSukhbir;
}

// 0x41B1D0
void Board::DoTypingCheck(const KeyCode theKey) {
    if (mApp->mKonamiCheck->Check(theKey)) {
        mApp->PlayFoley(FoleyType::FOLEY_DROP);
        return;
    }
    if (mApp->mMustacheCheck->Check(theKey) || mApp->mMoustacheCheck->Check(theKey)) {
        SetMustacheMode(!mBoardData.mMustacheMode);
        ReportAchievement::GiveAchievement(mApp, MustacheMode, false);
        // @Patoke: add achievement
        return;
    }
    if (mApp->mSuperMowerCheck->Check(theKey) || mApp->mSuperMowerCheck2->Check(theKey)) {
        SetSuperMowerMode(!mBoardData.mSuperMowerMode);
        return;
    }
    if (mApp->mFutureCheck->Check(theKey)) {
        SetFutureMode(!mBoardData.mFutureMode);
        return;
    }
    if (mApp->mPinataCheck->Check(theKey)) {
        if (mApp->CanDoPinataMode()) {
            SetPinataMode(!mBoardData.mPinataMode);
            return;
        }

        if (mApp->mGameScene == GameScenes::SCENE_PLAYING) {
            DisplayAdvice(_S("[CANT_USE_CODE]"), MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST, AdviceType::ADVICE_NONE);
        }
        mApp->PlaySample(Sexy::SOUND_BUZZER);
        return;
    }
    if (mApp->mDanceCheck->Check(theKey)) {
        if (mApp->CanDoDanceMode()) {
            SetDanceMode(!mBoardData.mDanceMode);
            return;
        }
        if (mApp->mGameScene == GameScenes::SCENE_PLAYING) {
            DisplayAdvice(_S("[CANT_USE_CODE]"), MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST, AdviceType::ADVICE_NONE);
        }
        mApp->PlaySample(Sexy::SOUND_BUZZER);
        return;
    }
    if (mApp->mDaisyCheck->Check(theKey)) {
        if (mApp->CanDoDaisyMode()) {
            SetDaisyMode(!mBoardData.mDaisyMode);
            return;
        }
        if (mApp->mGameScene == GameScenes::SCENE_PLAYING) {
            DisplayAdvice(_S("[CANT_USE_CODE]"), MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST, AdviceType::ADVICE_NONE);
        }
        mApp->PlaySample(Sexy::SOUND_BUZZER);
        return;
    }
    if (mApp->mSukhbirCheck->Check(theKey)) {
        SetSukhbirMode(!mBoardData.mSukhbirMode);
    }
}

// 0x41B820
void Board::KeyDown(const KeyCode theKey) {
    DoTypingCheck(theKey);

    if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO &&
        mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
        mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM) {
        mCutScene->KeyDown(theKey);
    } else if (theKey == KeyCode::KEYCODE_RETURN || theKey == KeyCode::KEYCODE_SPACE) {
        if (IsScaryPotterDaveTalking() && mApp->mCrazyDaveMessageIndex != -1) {
            mChallenge->AdvanceCrazyDaveDialog();
        } else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
            mApp->mZenGarden->AdvanceCrazyDaveDialog();
        } else if (theKey == KeyCode::KEYCODE_SPACE && mApp->CanPauseNow()) {
            mApp->PlaySample(Sexy::SOUND_PAUSE);
            mApp->DoPauseDialog();
        }
    } else if (theKey == KeyCode::KEYCODE_ESCAPE) {
        if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL) {
            RefreshSeedPacketFromCursor();
        } else if (CanInteractWithBoardButtons() && mApp->mGameScene != GameScenes::SCENE_ZOMBIES_WON) {
            mApp->DoNewOptions(false);
        }
    }
}

static void TodCrash() { TOD_ASSERT(false, "Crash%s", "!!!!"); }

// 0x41B950（原版中废弃）
void Board::KeyChar(const SexyChar theChar) {
    if (!mApp->mDebugKeysEnabled) return;

    TodTraceAndLog("Board cheat key '{}'", theChar);

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (theChar == _S('m')) {
            if (!mApp->mZenGarden->IsZenGardenFull(true)) {
                PottedPlant aPottedPlant;
                aPottedPlant.InitializePottedPlant(SeedType::SEED_MARIGOLD);
                aPottedPlant.mDrawVariation = static_cast<DrawVariation>(RandRangeInt(
                    (int)DrawVariation::VARIATION_MARIGOLD_WHITE, (int)DrawVariation::VARIATION_MARIGOLD_LIGHT_GREEN
                ));
                mApp->mZenGarden->AddPottedPlant(&aPottedPlant);
            }
            return;
        }

        if (theChar == _S('+')) {
            if (!mApp->mZenGarden->IsZenGardenFull(true)) {
                PottedPlant aPottedPlant;
                aPottedPlant.InitializePottedPlant(mApp->mZenGarden->PickRandomSeedType());
                mApp->mZenGarden->AddPottedPlant(&aPottedPlant);
            }
            return;
        }

        if (theChar == _S('a')) {
            if (!mApp->mZenGarden->IsZenGardenFull(true)) {
                PottedPlant aPottedPlant;
                aPottedPlant.InitializePottedPlant(mApp->mZenGarden->PickRandomSeedType());
                aPottedPlant.mPlantAge = PottedPlantAge::PLANTAGE_FULL;
                mApp->mZenGarden->AddPottedPlant(&aPottedPlant);
            }
            return;
        }

        if (theChar == _S('f')) {
            Plant *aPlant = nullptr;
            while (IteratePlants(aPlant)) {
                if (GetZenToolAt(aPlant->mPlantCol, aPlant->mRow) == nullptr && aPlant->mPottedPlantIndex >= 0) {
                    PottedPlant *aPottedPlant = mApp->mZenGarden->PottedPlantFromIndex(aPlant->mPottedPlantIndex);
                    const PottedPlantNeed aNeed = mApp->mZenGarden->GetPlantsNeed(aPottedPlant);
                    if (aNeed == PottedPlantNeed::PLANTNEED_WATER) {
                        aPlant->mHighlighted = true;
                        mApp->mZenGarden->MouseDownWithFeedingTool(
                            aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_WATERING_CAN
                        );
                        return;
                    } else if (aNeed == PottedPlantNeed::PLANTNEED_FERTILIZER) {
                        aPlant->mHighlighted = true;
                        if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_FERTILIZER) <= 0) {
                            mApp->mPlayerInfo->InitializePurchase(StoreItem::STORE_ITEM_FERTILIZER, 1);
                        }
                        mApp->mZenGarden->MouseDownWithFeedingTool(
                            aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_FERTILIZER
                        );
                        return;
                    } else if (aNeed == PottedPlantNeed::PLANTNEED_BUGSPRAY) {
                        aPlant->mHighlighted = true;
                        if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_BUG_SPRAY) <= 0) {
                            mApp->mPlayerInfo->InitializePurchase(StoreItem::STORE_ITEM_BUG_SPRAY, 1);
                        }
                        mApp->mZenGarden->MouseDownWithFeedingTool(
                            aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_BUG_SPRAY
                        );
                        return;
                    } else if (aNeed == PottedPlantNeed::PLANTNEED_PHONOGRAPH) {
                        aPlant->mHighlighted = true;
                        mApp->mZenGarden->MouseDownWithFeedingTool(
                            aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_PHONOGRAPH
                        );
                        return;
                    }
                }
            }
            return;
        }

        if (theChar == _S('r')) {
            Plant *aPlant = nullptr;
            while (IteratePlants(aPlant)) {
                if (aPlant->mPottedPlantIndex >= 0) {
                    TOD_ASSERT(aPlant->mPottedPlantIndex < mApp->mPlayerInfo->mNumPottedPlants);
                    PottedPlant *aPottedPlant = &mApp->mPlayerInfo->mPottedPlant[aPlant->mPottedPlantIndex];
                    mApp->mZenGarden->ResetPlantTimers(aPottedPlant);
                }
            }
            return;
        }

        if (theChar == _S('s')) {
            if (mApp->mZenGarden->IsStinkySleeping()) {
                mApp->mZenGarden->WakeStinky();
            } else {
                mApp->mZenGarden->ResetStinkyTimers();
            }
            return;
        }

        if (theChar == _S('c')) {
            if (!mApp->mPlayerInfo->hasPurchaseInitialized(StoreItem::STORE_ITEM_CHOCOLATE)) {
                mApp->mPlayerInfo->InitializePurchase(StoreItem::STORE_ITEM_CHOCOLATE, 1);
            } else {
                mApp->mPlayerInfo->UpdatePurchase(StoreItem::STORE_ITEM_CHOCOLATE, 1);
            }
            return;
        }

        if (theChar == _S(']')) {
            PottedPlant *aPottedPlant = mApp->mZenGarden->GetPottedPlantInWheelbarrow();
            if (aPottedPlant) {
                aPottedPlant->mSeedType = static_cast<SeedType>(static_cast<int>(aPottedPlant->mSeedType) + 1);
                if (aPottedPlant->mSeedType == SeedType::SEED_GATLINGPEA) {
                    aPottedPlant->mSeedType = SeedType::SEED_PEASHOOTER;
                }
                if (aPottedPlant->mSeedType == SeedType::SEED_FLOWERPOT) {
                    aPottedPlant->mSeedType = SeedType::SEED_KERNELPULT;
                }
            }
            return;
        }
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        if (theChar == _S('f')) {
            if (mApp->mPlayerInfo->GetPurchaseQuantity(StoreItem::STORE_ITEM_TREE_FOOD) <= 0) {
                mApp->mPlayerInfo->InitializePurchase(StoreItem::STORE_ITEM_TREE_FOOD, 1);
            }
            mChallenge->TreeOfWisdomFertilize();
        } else if (theChar == _S('g')) {
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('b')) {
            mChallenge->mChallengeStateCounter = 1;
        } else if (theChar == _S('0')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 0;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('1')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 9;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('2')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 19;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('3')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 29;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('4')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 39;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('5')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 49;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('6')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 98;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('7')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 498;
            mChallenge->TreeOfWisdomGrow();
        } else if (theChar == _S('8')) {
            mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 998;
            mChallenge->TreeOfWisdomGrow();
        }

        return;
    }

    if (theChar == _S('<')) {
        mApp->DoNewOptions(false);
    } else if (theChar == _S('l')) {
        mApp->DoCheatDialog();
    } else if (theChar == _S('#')) {
        if (mApp->IsSurvivalMode()) {
            if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO) {
                return;
            }

            mBoardData.mCurrentWave = mBoardData.mNumWaves;
            mChallenge->mSurvivalStage += 5;
            RemoveAllZombies();
            FadeOutLevel();
        }
    } else if (theChar == _S('!')) {
        mApp->mBoardResult = BoardResult::BOARDRESULT_CHEAT;
        if (IsLastStandStageWithRepick()) {
            if (mBoardData.mNextSurvivalStageCounter == 0) {
                mBoardData.mCurrentWave = mBoardData.mNumWaves;
                RemoveAllZombies();
                FadeOutLevel();
            }
        } else if ((mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage()) || mApp->IsEndlessIZombie(mApp->mGameMode)) {
            if (mBoardData.mNextSurvivalStageCounter == 0) {
                RemoveAllZombies();
                FadeOutLevel();
            }
        } else if (mApp->IsSurvivalMode()) {
            if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO) {
                return;
            }

            mBoardData.mCurrentWave = mBoardData.mNumWaves;
            // if (!IsSurvivalStageWithRepick())
            //{
            RemoveAllZombies();
            //}
            FadeOutLevel();
        } else {
            RemoveAllZombies();
            FadeOutLevel();
            mBoardData.mBoardFadeOutCounter = 200;
        }
    } else if (theChar == _S('+')) {
        mApp->mBoardResult = BoardResult::BOARDRESULT_CHEAT;
        if (IsLastStandStageWithRepick()) {
            if (mBoardData.mNextSurvivalStageCounter == 0) {
                mBoardData.mCurrentWave = mBoardData.mNumWaves;
                RemoveAllZombies();
                FadeOutLevel();
            }
        } else if ((mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage()) || mApp->IsEndlessIZombie(mApp->mGameMode)) {
            if (mBoardData.mNextSurvivalStageCounter == 0) {
                RemoveAllZombies();
                FadeOutLevel();
            }
        } else if (mApp->IsSurvivalEndless(mApp->mGameMode)) {
            if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO) {
                return;
            }

            mBoardData.mCurrentWave = mBoardData.mNumWaves;
            RemoveAllZombies();
            FadeOutLevel();
        } else if (mApp->IsSurvivalMode()) {
            mChallenge->mSurvivalStage = 5;
            RemoveAllZombies();
            FadeOutLevel();
            mBoardData.mBoardFadeOutCounter = 200;
        } else {
            RemoveAllZombies();
            FadeOutLevel();
            mBoardData.mBoardFadeOutCounter = 200;
        }
    } else if (theChar == _S('8')) {
        mApp->mEasyPlantingCheat = !mApp->mEasyPlantingCheat;
    } else if (theChar == _S('7')) {
        mApp->ToggleSlowMo();
    } else if (theChar == _S('6')) {
        mApp->ToggleFastMo();
    } else if (theChar == _S('z')) {
        mBoardData.mDebugTextMode = static_cast<DebugTextMode>(static_cast<int>(mBoardData.mDebugTextMode) + 1);
        if (mBoardData.mDebugTextMode > DebugTextMode::DEBUG_TEXT_COLLISION) {
            mBoardData.mDebugTextMode = DebugTextMode::DEBUG_TEXT_NONE;
        }
    }

    if (mApp->mGameScene != GameScenes::SCENE_PLAYING) {
        return;
    }

    Zombie *aBossZombie = GetBossZombie();
    if (aBossZombie && !aBossZombie->IsDeadOrDying()) {
        if (theChar == _S('b')) {
            aBossZombie->mBossBungeeCounter = 0;
            return;
        }
        if (theChar == _S('u')) {
            aBossZombie->mSummonCounter = 0;
            return;
        }
        if (theChar == _S('s')) {
            aBossZombie->mBossStompCounter = 0;
            return;
        }
        if (theChar == _S('r')) {
            aBossZombie->BossRVAttack();
            return;
        }
        if (theChar == _S('h')) {
            aBossZombie->mBossHeadCounter = 0;
            return;
        }
        if (theChar == _S('d')) {
            aBossZombie->TakeDamage(10000, 0U);
            return;
        }
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2) {
        if (theChar == _S('w')) {
            AddZombie(ZombieType::ZOMBIE_WALLNUT_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
            return;
        }
        if (theChar == _S('t')) {
            AddZombie(ZombieType::ZOMBIE_TALLNUT_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
            return;
        }
        if (theChar == _S('j')) {
            AddZombie(ZombieType::ZOMBIE_JALAPENO_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
            return;
        }
        if (theChar == _S('g')) {
            AddZombie(ZombieType::ZOMBIE_GATLING_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
            return;
        }
        if (theChar == _S('s')) {
            AddZombie(ZombieType::ZOMBIE_SQUASH_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
            return;
        }
    }

    if (theChar == _S('q')) {
        if (mApp->IsSurvivalEndless(mApp->mGameMode)) {
            mApp->mEasyPlantingCheat = true;
            for (int y = 0; y < MAX_GRID_SIZE_X; y++) {
                for (int x = 0; x < MAX_GRID_SIZE_Y; x++) {
                    if (CanPlantAt(x, y, SeedType::SEED_LILYPAD) == PlantingReason::PLANTING_OK) {
                        AddPlant(x, y, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
                    }
                    if (CanPlantAt(x, y, SeedType::SEED_PUMPKINSHELL) == PlantingReason::PLANTING_OK) {
                        if (x <= 6 || IsPoolSquare(x, y)) {
                            AddPlant(x, y, SeedType::SEED_PUMPKINSHELL, SeedType::SEED_NONE);
                        }
                    }
                    if (CanPlantAt(x, y, SeedType::SEED_GATLINGPEA) == PlantingReason::PLANTING_OK) {
                        if (x < 5) {
                            AddPlant(x, y, SeedType::SEED_GATLINGPEA, SeedType::SEED_NONE);
                        } else if (x == 5) {
                            AddPlant(x, y, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
                        } else if (x == 6) {
                            AddPlant(x, y, SeedType::SEED_SPLITPEA, SeedType::SEED_NONE);
                        } else if (y == 2 || y == 3) {
                            AddPlant(x, y, SeedType::SEED_GLOOMSHROOM, SeedType::SEED_NONE);
                            if (CanPlantAt(x, y, SeedType::SEED_INSTANT_COFFEE) == PlantingReason::PLANTING_OK) {
                                AddPlant(x, y, SeedType::SEED_INSTANT_COFFEE, SeedType::SEED_NONE);
                            }
                        }
                    }
                }
            }
        } else if (mApp->IsIZombieLevel()) {
            mApp->mEasyPlantingCheat = true;
            for (int i = 0; i < 5; i++) {
                mChallenge->IZombiePlaceZombie(ZombieType::ZOMBIE_FOOTBALL, 6, i);
            }
        } else {
            mApp->mEasyPlantingCheat = true;
            for (int y = 0; y < MAX_GRID_SIZE_Y; ++y) {
                for (int x = 0; x < MAX_GRID_SIZE_X; ++x) {
                    if (StageHasRoof() && CanPlantAt(x, y, SeedType::SEED_FLOWERPOT) == PlantingReason::PLANTING_OK) {
                        AddPlant(x, y, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
                    }
                    if (CanPlantAt(x, y, SeedType::SEED_LILYPAD) == PlantingReason::PLANTING_OK) {
                        AddPlant(x, y, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
                    }
                    if (CanPlantAt(x, y, SeedType::SEED_THREEPEATER) == PlantingReason::PLANTING_OK) {
                        AddPlant(x, y, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
                    }
                }
            }

            if (!mChallenge->UpdateZombieSpawning()) {
                int aWavesRemaining = std::min(mBoardData.mNumWaves - mBoardData.mCurrentWave, 20);
                while (aWavesRemaining) {
                    SpawnZombieWave();
                    aWavesRemaining--;
                }
            }

            if (mApp->IsScaryPotterLevel()) {
                GridItem *aGridItem = nullptr;
                while (IterateGridItems(aGridItem)) {
                    if (aGridItem->mGridItemType == GridItemType::GRIDITEM_SCARY_POT) {
                        mChallenge->ScaryPotterOpenPot(aGridItem);
                    }
                }
            }
        }

        return;
    }

    if (theChar == _S('O')) {
        mApp->mEasyPlantingCheat = true;
        for (int y = 0; y < MAX_GRID_SIZE_Y; y++) {
            for (int x = 0; x < 3; x++) {
                if (CanPlantAt(x, y, SeedType::SEED_FLOWERPOT) == PlantingReason::PLANTING_OK) {
                    AddPlant(x, y, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
                }
            }
        }
        return;
    }

    if (theChar == _S('?') || theChar == _S('/')) {
        if (mBoardData.mHugeWaveCountDown > 0) {
            mBoardData.mHugeWaveCountDown = 1;
        } else {
            mBoardData.mZombieCountDown = 6;
        }
        return;
    }

    if (theChar == _S('b')) {
        AddZombie(ZombieType::ZOMBIE_BUNGEE, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('o')) {
        AddZombie(ZombieType::ZOMBIE_FOOTBALL, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('s')) {
        AddZombie(ZombieType::ZOMBIE_DOOR, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('L')) {
        AddZombie(ZombieType::ZOMBIE_LADDER, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('y')) {
        AddZombie(ZombieType::ZOMBIE_YETI, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('a')) {
        AddZombie(ZombieType::ZOMBIE_FLAG, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('w')) {
        AddZombie(ZombieType::ZOMBIE_NEWSPAPER, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('F')) {
        AddZombie(ZombieType::ZOMBIE_BALLOON, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('n')) {
        if (StageHasPool()) {
            AddZombie(ZombieType::ZOMBIE_SNORKEL, Zombie::ZOMBIE_WAVE_DEBUG);
        }
    }
    if (theChar == _S('c')) {
        AddZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('m')) {
        AddZombie(ZombieType::ZOMBIE_DANCER, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('h')) {
        AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    // if (theChar == _S('H')
    //{
    //	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
    //	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
    //	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
    //	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
    //	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
    //	return;
    // }
    if (theChar == _S('D')) {
        AddZombie(ZombieType::ZOMBIE_DIGGER, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('p')) {
        AddZombie(ZombieType::ZOMBIE_POLEVAULTER, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('P')) {
        AddZombie(ZombieType::ZOMBIE_POGO, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('R')) {
        if (StageHasPool()) {
            AddZombie(ZombieType::ZOMBIE_DOLPHIN_RIDER, Zombie::ZOMBIE_WAVE_DEBUG);
        }
        return;
    } else if (theChar == _S('j')) {
        AddZombie(ZombieType::ZOMBIE_JACK_IN_THE_BOX, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('g')) {
        AddZombie(ZombieType::ZOMBIE_GARGANTUAR, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('G')) {
        AddZombie(ZombieType::ZOMBIE_REDEYE_GARGANTUAR, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('i')) {
        AddZombie(ZombieType::ZOMBIE_ZAMBONI, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('C')) {
        AddZombie(ZombieType::ZOMBIE_CATAPULT, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('1')) {
        Plant *aPlant = GetTopPlantAt(0, 0, PlantPriority::TOPPLANT_ANY);
        if (aPlant) {
            aPlant->Die();
            mChallenge->ZombieAtePlant(aPlant);
            return;
        }
    }
    if (theChar == _S('B')) {
        mBoardData.mFogBlownCountDown = 2200;
        return;
    }
    if (theChar == _S('t')) {
        if (!CanAddBobSled()) {
            int aRow = Rand(5);
            int aPos = 400;
            if (StageHasPool()) {
                aRow = Rand(2);
            } else if (StageHasRoof()) {
                aPos = 500;
            }
            mBoardData.mIceTimer[aRow] = 3000;
            mBoardData.mIceMinX[aRow] = aPos;
        }

        AddZombie(ZombieType::ZOMBIE_BOBSLED, Zombie::ZOMBIE_WAVE_DEBUG);
        return;
    }
    if (theChar == _S('r')) {
        SpawnZombiesFromGraves();
        return;
    }
    if (theChar == _S('0')) {
        AddSunMoney(100);
        mApp->PlaySample(SOUND_BUTTONCLICK);
        return;
    }
    if (theChar == _S('9')) {
        AddSunMoney(999999);
        mApp->PlaySample(SOUND_BUTTONCLICK);
        return;
    }
    if (theChar == _S('$')) {
        mApp->mPlayerInfo->AddCoins(100);
        mApp->PlaySample(SOUND_BUTTONCLICK);
        ShowCoinBank();
        return;
    }
    if (theChar == _S('-')) {
        mBoardData.mSunMoney -= 100;
        if (mBoardData.mSunMoney < 0) {
            mBoardData.mSunMoney = 0;
        }
        return;
    }
    if (theChar == _S('%')) {
        mApp->SwitchScreenMode(mApp->mIsWindowed, false, false);
    }
    if (theChar == _S('M')) {
        mApp->mMusic->mBurstOverride -= 2 - (mApp->mMusic->mBurstOverride != 1);
        return;
    }

    if (theChar == _S('\3') && mApp->mCtrlDown && mApp->mTodCheatKeys) {
        TodCrash();

        if (mBoardData.mHugeWaveCountDown > 0) {
            mBoardData.mHugeWaveCountDown = 1;
        } else {
            mBoardData.mZombieCountDown = 6;
        }
    }
}

// 0x41B960
//  GOTY @Patoke: 0x41E6E0
void Board::AddSunMoney(const int theAmount) {
    mBoardData.mSunMoney += theAmount;
    if (mBoardData.mSunMoney > 9990) {
        mBoardData.mSunMoney = 9990;
    }
    if (mBoardData.mSunMoney >= 8000)
        // if ( !*(mApp->mPlayerInfo + 48) ) todo @Patoke: figure this out
        ReportAchievement::GiveAchievement(mApp, SunnyDays, true);
}

// 0x41B980
int Board::CountSunBeingCollected() {
    int aCount = 0;
    Coin *aCoin = nullptr;
    while (IterateCoins(aCoin)) {
        if (aCoin->mIsBeingCollected && aCoin->IsSun()) {
            aCount += aCoin->GetSunValue();
        }
    }
    return aCount;
}

// 0x41B9F0
int Board::CountCoinsBeingCollected() {
    int aCount = 0;
    Coin *aCoin = nullptr;
    while (IterateCoins(aCoin)) {
        if (aCoin->mIsBeingCollected && aCoin->IsMoney()) {
            aCount += aCoin->GetCoinValue(aCoin->mType);
        }
    }
    return aCount;
}

// 0x41BA60
bool Board::TakeSunMoney(const int theAmount) {
    if (CanTakeSunMoney(theAmount)) {
        mBoardData.mSunMoney -= theAmount;
        return true;
    }

    mApp->PlaySample(Sexy::SOUND_BUZZER);
    mBoardData.mOutOfMoneyCounter = 70;
    return false;
}

// 0x41BAB0
bool Board::CanTakeSunMoney(const int theAmount) {
    return theAmount <= mBoardData.mSunMoney + CountSunBeingCollected();
}

// 0x41BAD0
void Board::ProcessDeleteQueue() {
    {
        Plant *aPlant = nullptr;
        while (mPlants.IterateNext(aPlant)) {
            if (aPlant->mDead) {
                mPlants.DataArrayFree(aPlant);
            }
        }
    }
    {
        Zombie *aZombie = nullptr;
        while (mZombies.IterateNext(aZombie)) {
            if (aZombie->mDead) {
                mZombies.DataArrayFree(aZombie);
            }
        }
    }
    {
        Projectile *aProjectile = nullptr;
        while (mProjectiles.IterateNext(aProjectile)) {
            if (aProjectile->mDead) {
                mProjectiles.DataArrayFree(aProjectile);
            }
        }
    }
    {
        Coin *aCoin = nullptr;
        while (mCoins.IterateNext(aCoin)) {
            if (aCoin->mDead) {
                mCoins.DataArrayFree(aCoin);
            }
        }
    }
    {
        LawnMower *aLawnMower = nullptr;
        while (mLawnMowers.IterateNext(aLawnMower)) {
            if (aLawnMower->mDead) {
                mLawnMowers.DataArrayFree(aLawnMower);
            }
        }
    }
    {
        GridItem *aGridItem = nullptr;
        while (mGridItems.IterateNext(aGridItem)) {
            if (aGridItem->mDead) {
                mGridItems.DataArrayFree(aGridItem);
            }
        }
    }
}

// 0x41BE50
//  GOTY @Patoke: 0x41EC10
bool Board::HasConveyorBeltSeedBank() {
    return mApp->IsFinalBossLevel() || mApp->IsMiniBossLevel() || mApp->IsShovelLevel() ||
           mApp->IsWallnutBowlingLevel() || mApp->IsLittleTroubleLevel() || mApp->IsStormyNightLevel() ||
           mApp->IsBungeeBlitzLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT ||
           mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN ||
           mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL;
}

// 0x41BEE0
int Board::GetNumSeedsInBank() {
    if (mApp->IsScaryPotterLevel()) {
        return 1;
    }
    if (mApp->IsWhackAZombieLevel()) {
        return 3;
    }
    if (mApp->IsChallengeWithoutSeedBank()) {
        return 0;
    }
    if (HasConveyorBeltSeedBank()) {
        return 10;
    }
    if (mApp->IsSlotMachineLevel()) {
        return 3;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE) {
        return 6;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST) {
        return 0;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM) {
        return 2;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1 ||
        mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2 ||
        mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3 ||
        mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4) {
        return 3;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5 ||
        mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6 ||
        mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7) {
        return 4;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8) {
        return 6;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9) {
        return 8;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS) {
        return 9;
    }

    const int aNumSeeds = mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_PACKET_UPGRADE)] + 6;
    const int aSeedsAvailable = mApp->GetSeedsAvailable();
    return std::min(aNumSeeds, aSeedsAvailable);
}

// 0x41C010
bool Board::StageIsNight() {
    return mBoardData.mBackground == BackgroundType::BACKGROUND_2_NIGHT ||
           mBoardData.mBackground == BackgroundType::BACKGROUND_4_FOG ||
           mBoardData.mBackground == BackgroundType::BACKGROUND_6_BOSS ||
           mBoardData.mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN ||
           mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM;
}

// 0x41C040
bool Board::StageHasGraveStones() {
    if (mApp->IsWallnutBowlingLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_POGO_PARTY ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mApp->IsIZombieLevel() ||
        mApp->IsScaryPotterLevel())
        return false;

    return mBoardData.mBackground == BackgroundType::BACKGROUND_2_NIGHT;
}

// 0x41C0B0
bool Board::StageHasRoof() const {
    return (
        mBoardData.mBackground == BackgroundType::BACKGROUND_5_ROOF ||
        mBoardData.mBackground == BackgroundType::BACKGROUND_6_BOSS
    );
}

// 0x41C0D0
bool Board::StageHasPool() const {
    return (
        mBoardData.mBackground == BackgroundType::BACKGROUND_3_POOL ||
        mBoardData.mBackground == BackgroundType::BACKGROUND_4_FOG
    );
}

bool Board::StageHas6Rows() const {
    return (
        mBoardData.mBackground == BackgroundType::BACKGROUND_3_POOL ||
        mBoardData.mBackground == BackgroundType::BACKGROUND_4_FOG
    );
}

// 0x41C0F0
bool Board::StageHasZombieWalkInFromRight() const {
    if (mApp->IsWhackAZombieLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
        mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || mApp->IsFinalBossLevel() ||
        mApp->IsIZombieLevel() || mApp->IsSquirrelLevel() || mApp->IsScaryPotterLevel())
        return false;

    return true;
}

// 0x41C170
bool Board::StageHasFog() const {
    return !mApp->IsStormyNightLevel() && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL &&
           mBoardData.mBackground == BackgroundType::BACKGROUND_4_FOG;
}

// GOTY @Patoke: inlined 0x41E669
bool Board::StageIsDayWithoutPool() const { return mBoardData.mBackground == BackgroundType::BACKGROUND_1_DAY; }

// GOTY @Patoke: inlined 0x41E5E6
bool Board::StageIsDayWithPool() const { return mBoardData.mBackground == BackgroundType::BACKGROUND_3_POOL; }

// 0x41C1C0
int Board::LeftFogColumn() const {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_AIR_RAID) return 6;
    if (!mApp->IsAdventureMode()) return 5;
    if (mBoardData.mLevel == 31) return 6;
    if (mBoardData.mLevel >= 32 && mBoardData.mLevel <= 36) return 5;
    if (mBoardData.mLevel >= 37 && mBoardData.mLevel <= 40) return 4;
    TOD_ASSERT();

    unreachable();
}

// 0x41C210
int Board::GetSeedPacketPositionX(const int theIndex) {
    if (mApp->IsSlotMachineLevel()) return theIndex * 59 + 247;
    if (HasConveyorBeltSeedBank()) return theIndex * 50 + 91;

    if (mSeedBank->mNumPackets <= 7) return theIndex * 59 + 85;
    else if (mSeedBank->mNumPackets == 8) return theIndex * 54 + 81;
    else if (mSeedBank->mNumPackets == 9) return theIndex * 52 + 80;
    else return theIndex * 51 + 79;
}

// 0x41C290
int Board::GetSeedBankExtraWidth() const {
    const int aNumPackets = mSeedBank->mNumPackets;
    return aNumPackets <= 6 ? 0 : aNumPackets == 7 ? 60 : aNumPackets == 8 ? 76 : aNumPackets == 9 ? 112 : 153;
}

void Board::OffsetYForPlanting(int &theY, const SeedType theSeedType) const {
    if (Plant::IsFlying(theSeedType) || theSeedType == SeedType::SEED_GRAVEBUSTER) {
        theY += 15;
    }
    if (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK) {
        theY -= 15;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
        mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE) {
        theY -= 25;
    }
}

// 0x41C2D0
int Board::PlantingPixelToGridX(const int theX, int theY, const SeedType theSeedType) const {
    OffsetYForPlanting(theY, theSeedType);
    return PixelToGridX(theX, theY);
}

// 0x41C320
int Board::PlantingPixelToGridY(const int theX, int theY, const SeedType theSeedType) {
    OffsetYForPlanting(theY, theSeedType);

    const int aGridY = PixelToGridY(theX, theY);
    if (theSeedType == SeedType::SEED_INSTANT_COFFEE) {
        const int aGridX = PixelToGridX(theX, theY);

        const Plant *aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
        if (aPlant && aPlant->mIsAsleep) {
            return aGridY;
        }

        const int aGridYDown = PixelToGridY(theX, theY + 30);
        if (aGridYDown != aGridY) {
            const Plant *aPlantDown = GetTopPlantAt(aGridX, aGridYDown, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
            if (aPlantDown && aPlantDown->mIsAsleep) {
                return aGridYDown;
            }
        }

        const int aGridYUp = PixelToGridY(theX, theY - 50);
        if (aGridYUp != aGridY) {
            const Plant *aPlantUp = GetTopPlantAt(aGridX, aGridYUp, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
            if (aPlantUp && aPlantUp->mIsAsleep) {
                return aGridYUp;
            }
        }
    }
    return aGridY;
}

// 0x41C4C0
int Board::PixelToGridX(const int theX, const int theY) const {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
            return mApp->mZenGarden->PixelToGridX(theX, theY);
        }
    }

    if (theX < LAWN_XMIN) return -1;

    return ClampInt((theX - LAWN_XMIN) / 80, 0, MAX_GRID_SIZE_X - 1);
}

// 0x41C530
int Board::PixelToGridXKeepOnBoard(const int theX, const int theY) const {
    const int aGridX = PixelToGridX(theX, theY);
    return std::max(aGridX, 0);
}

// 0x41C550
int Board::PixelToGridY(const int theX, int theY) const {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
            return mApp->mZenGarden->PixelToGridY(theX, theY);
        }
    }

    const int aGridX = PixelToGridX(theX, theY);
    if (aGridX == -1 || theY < LAWN_YMIN) return -1;

    if (StageHasRoof()) {
        if (aGridX < 5) {
            theY -= (4 - aGridX) * 20;
        }
        return ClampInt((theY - LAWN_YMIN) / 85, 0, MAX_GRID_SIZE_Y - 2);
    } else if (StageHasPool()) {
        return ClampInt((theY - LAWN_YMIN) / 85, 0, MAX_GRID_SIZE_Y - 1);
    } else {
        return ClampInt((theY - LAWN_YMIN) / 100, 0, MAX_GRID_SIZE_Y - 2);
    }
}

// 0x41C650
int Board::PixelToGridYKeepOnBoard(const int theX, const int theY) {
    const int aGridY = PixelToGridY(std::max(theX, 80), theY);
    return std::max(aGridY, 0);
}

// 0x41C680
int Board::GridToPixelX(const int theGridX, const int theGridY) {
    TOD_ASSERT(theGridX >= 0 && theGridX < MAX_GRID_SIZE_X);
    TOD_ASSERT(theGridY >= 0 && theGridY < MAX_GRID_SIZE_Y);
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
            return mApp->mZenGarden->GridToPixelX(theGridX, theGridY);
        }
    }

    return theGridX * 80 + LAWN_XMIN;
}

// 0x41C6C0
float Board::GetPosYBasedOnRow(const float thePosX, const int theRow) {
    if (StageHasRoof()) {
        float aSlopeOffset = 0.0f;
        if (thePosX < 440.0f) {
            aSlopeOffset = (440.0f - thePosX) * 0.25f;
        }

        return GridToPixelY(8, theRow) + aSlopeOffset;
    }

    return GridToPixelY(0, theRow);
}

// 0x41C740
int Board::GridToPixelY(const int theGridX, const int theGridY) {
    TOD_ASSERT(theGridX >= 0 && theGridX < MAX_GRID_SIZE_X);
    TOD_ASSERT(theGridY >= 0 && theGridY < MAX_GRID_SIZE_Y);
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        if (mBoardData.mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN ||
            mBoardData.mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM) {
            return mApp->mZenGarden->GridToPixelY(theGridX, theGridY);
        }
    }

    int aY;
    if (StageHasRoof()) {
        int aSlopeOffset;
        if (theGridX < 5) {
            aSlopeOffset = (5 - theGridX) * 20;
        } else {
            aSlopeOffset = 0;
        }
        aY = theGridY * 85 + aSlopeOffset + LAWN_YMIN - 10;
    } else if (StageHasPool()) {
        aY = theGridY * 85 + LAWN_YMIN;
    } else {
        aY = theGridY * 100 + LAWN_YMIN;
    }

    if (theGridX != -1 && mBoardData.mGridSquareType[theGridX][theGridY] == GridSquareType::GRIDSQUARE_HIGH_GROUND) {
        aY -= HIGH_GROUND_HEIGHT;
    }

    return aY;
}

ZombieID Board::ZombieGetID(Zombie *theZombie) { return static_cast<ZombieID>(mZombies.DataArrayGetID(theZombie)); }

Zombie *Board::ZombieGet(const ZombieID theZombieID) { return mZombies.DataArrayGet((unsigned int)theZombieID); }

// 0x41C7F0
Zombie *Board::ZombieTryToGet(const ZombieID theZombieID) {
    return mZombies.DataArrayTryToGet((unsigned int)theZombieID);
}

// 0x41C820
int GetRectOverlap(const Rect &rect1, const Rect &rect2) {
    int xmax, rmin, rmax;

    if (rect1.mX < rect2.mX) {
        rmin = rect1.mX + rect1.mWidth;
        rmax = rect2.mX + rect2.mWidth;
        xmax = rect2.mX;
    } else {
        rmin = rect2.mX + rect2.mWidth;
        rmax = rect1.mX + rect1.mWidth;
        xmax = rect1.mX;
    }

    if (rmin > xmax && rmin > rmax) {
        rmin = rmax;
    }

    return rmin - xmax;
}

// 0x41C850
bool GetCircleRectOverlap(const int theCircleX, const int theCircleY, const int theRadius, const Rect &theRect) {
    int dx = 0;        // 圆心与矩形较近一条纵边的横向距离
    int dy = 0;        // 圆心与矩形较近一条横边的纵向距离
    bool xOut = false; // 圆心横坐标是否不在矩形范围内
    bool yOut = false; // 圆心纵坐标是否不在矩形范围内

    if (theCircleX < theRect.mX) {
        xOut = true;
        dx = theRect.mX - theCircleX;
    } else if (theCircleX > theRect.mX + theRect.mWidth) {
        xOut = true;
        dx = theCircleX - theRect.mX - theRect.mWidth;
    }
    if (theCircleY < theRect.mY) {
        yOut = true;
        dy = theRect.mY - theCircleY;
    } else if (theCircleY > theRect.mY + theRect.mHeight) {
        yOut = true;
        dy = theCircleY - theRect.mY - theRect.mHeight;
    }

    if (!xOut && !yOut) // 如果圆心在矩形内
    {
        return true;
    } else if (xOut && yOut) {
        return dx * dx + dy * dy <= theRadius * theRadius;
    } else if (xOut) {
        return dx <= theRadius;
    } else {
        return dy <= theRadius;
    }
}

// 0x41C8F0
//  GOTY @Patoke: 0x41F6B0
bool Board::IterateZombies(Zombie *&theZombie) {
    while (mZombies.IterateNext(theZombie)) {
        if (!theZombie->mDead) {
            return true;
        }
    }

    theZombie = reinterpret_cast<Zombie *>(-1);
    return false;
}

// 0x41C950
bool Board::IteratePlants(Plant *&thePlant) {
    while (mPlants.IterateNext(thePlant)) {
        if (!thePlant->mDead) {
            return true;
        }
    }

    thePlant = reinterpret_cast<Plant *>(-1);
    return false;
}

// 0x41C9B0
bool Board::IterateProjectiles(Projectile *&theProjectile) {
    while (mProjectiles.IterateNext(theProjectile)) {
        if (!theProjectile->mDead) {
            return true;
        }
    }

    theProjectile = reinterpret_cast<Projectile *>(-1);
    return false;
}

// 0x41CA10
bool Board::IterateCoins(Coin *&theCoin) {
    while (mCoins.IterateNext(theCoin)) {
        if (!theCoin->mDead) {
            return true;
        }
    }

    theCoin = reinterpret_cast<Coin *>(-1);
    return false;
}

// 0x41CA70
bool Board::IterateLawnMowers(LawnMower *&theLawnMower) {
    while (mLawnMowers.IterateNext(theLawnMower)) {
        if (!theLawnMower->mDead) {
            return true;
        }
    }

    theLawnMower = reinterpret_cast<LawnMower *>(-1);
    return false;
}

// 0x41CAD0
bool Board::IterateGridItems(GridItem *&theGridItem) {
    while (mGridItems.IterateNext(theGridItem)) {
        if (!theGridItem->mDead) {
            return true;
        }
    }

    theGridItem = reinterpret_cast<GridItem *>(-1);
    return false;
}

// 0x41CB30
bool Board::IterateParticles(TodParticleSystem *&theParticle) {
    while (mApp->mEffectSystem->mParticleHolder->mParticleSystems.IterateNext(theParticle)) {
        if (!theParticle->mDead) {
            return true;
        }
    }

    theParticle = reinterpret_cast<TodParticleSystem *>(-1);
    return false;
}

bool Board::IterateReanimations(Reanimation *&theReanimation) {
    while (mApp->mEffectSystem->mReanimationHolder->mReanimations.IterateNext(theReanimation)) {
        if (!theReanimation->mDead) {
            return true;
        }
    }

    theReanimation = reinterpret_cast<Reanimation *>(-1);
    return false;
}

// 0x41CBF0
void Board::KillAllPlantsInRadius(const int theX, const int theY, const int theRadius) {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (GetCircleRectOverlap(theX, theY, theRadius, aPlant->GetPlantRect())) {
            mBoardData.mPlantsEaten++;
            aPlant->Die();
        }
    }
}

// 0x41CC60
unsigned int Board::SeedNotRecommendedForLevel(const SeedType theSeedType) {
    unsigned int aNotRec = 0;
    if (Plant::IsNocturnal(theSeedType) && !StageIsNight()) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NOCTURNAL, true);
    }
    if (theSeedType == SeedType::SEED_INSTANT_COFFEE && StageIsNight()) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_AT_NIGHT, true);
    }
    if (theSeedType == SeedType::SEED_GRAVEBUSTER && !StageHasGraveStones()) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_GRAVES, true);
    }
    if (theSeedType == SeedType::SEED_PLANTERN && !StageHasFog()) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_FOG, true);
    }
    if (theSeedType == SeedType::SEED_FLOWERPOT && !StageHasRoof()) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_ROOF, true);
    }
    if (StageHasRoof() && (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK)) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_ON_ROOF, true);
    }
    if (!StageHasPool() && Plant::IsAquatic(theSeedType)) {
        SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_POOL, true);
    }
    return aNotRec;
}

// 0x41CD70
int Board::CountCoinByType(const CoinType theCoinType) {
    int aCount = 0;

    Coin *aCoin = nullptr;
    while (IterateCoins(aCoin)) {
        if (aCoin->mType == theCoinType) {
            aCount++;
        }
    }

    return aCount;
}

int Board::GetGraveStoneCount() {
    int aCount = 0;

    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE) {
            aCount++;
        }
    }

    return aCount;
}

// 0x41CDB0
void Board::DropLootPiece(const int thePosX, const int thePosY, const int theDropFactor) {
    if (mApp->IsFirstTimeAdventureMode()) {
        if (mBoardData.mLevel == 22 && mBoardData.mCurrentWave > 5 && !mApp->mPlayerInfo->mHasUnlockedMinigames &&
            CountCoinByType(CoinType::COIN_PRESENT_MINIGAMES) == 0) {
            mApp->PlayFoley(FoleyType::FOLEY_ART_CHALLENGE);
            AddCoin(thePosX, thePosY, CoinType::COIN_PRESENT_MINIGAMES, CoinMotion::COIN_MOTION_COIN);
            return;
        }
        if (mBoardData.mLevel == 36 && mBoardData.mCurrentWave > 5 && !mApp->mPlayerInfo->mHasUnlockedPuzzleMode &&
            CountCoinByType(CoinType::COIN_PRESENT_PUZZLE_MODE) == 0) {
            mApp->PlayFoley(FoleyType::FOLEY_ART_CHALLENGE);
            AddCoin(thePosX, thePosY, CoinType::COIN_PRESENT_PUZZLE_MODE, CoinMotion::COIN_MOTION_COIN);
            return;
        }
    }

    int aDropHit = Rand(30000);
    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 11 && !mBoardData.mDroppedFirstCoin &&
        mBoardData.mCurrentWave > 5) {
        aDropHit = 1000;
    }
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN) {
        aDropHit *= 5;
    }

    if (mApp->IsWhackAZombieLevel()) {
        constexpr int aSunChanceMin = 2500;
        const int aSunChanceMax = mBoardData.mSunMoney > 500   ? 2800
                                  : mBoardData.mSunMoney > 350 ? 3100
                                  : mBoardData.mSunMoney > 200 ? 3700
                                                               : 5000;
        if (aDropHit >= aSunChanceMin * theDropFactor && aDropHit <= aSunChanceMax * theDropFactor) {
            mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
            AddCoin(thePosX - 20, thePosY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_COIN);
            AddCoin(thePosX - 40, thePosY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_COIN);
            AddCoin(thePosX - 60, thePosY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_COIN);
            return;
        }
    }

    if (mBoardData.mTotalSpawnedWaves > 70) return;

    int aPottedPlantChance;
    if (!mApp->mZenGarden->CanDropPottedPlantLoot()) {
        aPottedPlantChance = 0;
    } else if (mApp->IsAdventureMode() && !mApp->IsFirstTimeAdventureMode()) {
        aPottedPlantChance = 24;
    } else {
        aPottedPlantChance = mApp->IsSurvivalEndless(mApp->mGameMode) ? 3 : 12;
    }

    int aChocolateChance = aPottedPlantChance;
    if (mApp->mZenGarden->CanDropChocolate()) {
        if (mApp->IsAdventureMode() && !mApp->IsFirstTimeAdventureMode()) {
            aChocolateChance = aPottedPlantChance + 72;
        } else {
            aChocolateChance = aPottedPlantChance + (mApp->IsSurvivalEndless(mApp->mGameMode) ? 9 : 36);
        }
    }

    const int aDiamondChance = aChocolateChance + 14;
    const int aGoldChance = aChocolateChance + 250;
    const int aSilverChance = aChocolateChance + 2500;

    CoinType aCoinType;
    if (aDropHit < aPottedPlantChance * theDropFactor) {
        aCoinType = CoinType::COIN_PRESENT_PLANT;
    } else if (aDropHit < aChocolateChance * theDropFactor) {
        aCoinType = CoinType::COIN_CHOCOLATE;
    } else if (aDropHit < aDiamondChance * theDropFactor) {
        aCoinType = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] < 1 ? CoinType::COIN_GOLD
                                                                                            : CoinType::COIN_DIAMOND;
    } else if (aDropHit < aGoldChance * theDropFactor) {
        aCoinType = CoinType::COIN_GOLD;
    } else if (aDropHit < aSilverChance * theDropFactor) {
        aCoinType = CoinType::COIN_SILVER;
    } else return;

    if (mApp->IsWallnutBowlingLevel() && Coin::IsMoney(aCoinType)) return;

    if (mApp->IsFirstTimeAdventureMode() && mBoardData.mLevel == 11) {
        int aMoney = Coin::GetCoinValue(CoinType::COIN_GOLD) * mLawnMowers.mSize;
        const int aCost = StoreScreen::GetItemCost(StoreItem::STORE_ITEM_PACKET_UPGRADE);
        aMoney += mApp->mPlayerInfo->mCoins + CountCoinsBeingCollected();
        if (Coin::GetCoinValue(aCoinType) + aMoney >= aCost) {
            return;
        }
    }

    mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
    AddCoin(thePosX - 40, thePosY, aCoinType, CoinMotion::COIN_MOTION_COIN);
    mBoardData.mDroppedFirstCoin = true;
}

// 0x41D2C0
bool Board::CanDropLoot() {
    return !mCutScene->ShouldRunUpsellBoard() && (!mApp->IsFirstTimeAdventureMode() || mBoardData.mLevel >= 11);
}

// 0x41D320
bool Board::BungeeIsTargetingCell(const int theGridX, const int theGridY) {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (!aZombie->IsDeadOrDying() && aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE &&
            aZombie->mRow == theGridY && aZombie->mTargetCol == theGridX) {
            return true;
        }
    }
    return false;
}

// 0x41D390
Zombie *Board::GetBossZombie() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) {
            return aZombie;
        }
    }
    return nullptr;
}

// 0x41D3D0
Plant *Board::FindUmbrellaPlant(const int theGridX, const int theGridY) {
    Plant *aPlant = nullptr;
    while (IteratePlants(aPlant)) {
        if (aPlant->mSeedType == SeedType::SEED_UMBRELLA && !aPlant->NotOnGround() &&
            GridInRange(theGridX, theGridY, aPlant->mPlantCol, aPlant->mRow, 1, 1)) {
            return aPlant;
        }
    }
    return nullptr;
}

// 0x41D450
void Board::DoFwoosh(const int theRow) {
    const int aRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, theRow, 1);
    for (int i = 0; i < 12; i++) {
        Reanimation *aOriReanim = mApp->ReanimationTryToGet(mBoardData.mFwooshID[theRow][i]);
        if (aOriReanim) {
            aOriReanim->ReanimationDie();
        }

        const float aPosX = 750.0f * i / 11.0f + 10.0f;
        const float aPosY = GetPosYBasedOnRow(aPosX + 10.0f, theRow) - 10.0f;
        Reanimation *aFwoosh = mApp->AddReanimation(aPosX, aPosY, aRenderOrder, ReanimationType::REANIM_JALAPENO_FIRE);
        aFwoosh->SetFramesForLayer("anim_flame");
        aFwoosh->mLoopType = ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME;
        aFwoosh->mAnimRate *= RandRangeFloat(0.7f, 1.3f);

        const float aScale = RandRangeFloat(0.9f, 1.1f);
        const float aFlip = Rand(2) ? 1.0f : -1.0f;
        aFwoosh->OverrideScale(aScale * aFlip, 1);

        mBoardData.mFwooshID[theRow][i] = mApp->ReanimationGetID(aFwoosh);
    }
    mBoardData.mFwooshCountDown = 100;
}

// 0x41D630
void Board::UpdateFwoosh() {
    if (mBoardData.mFwooshCountDown == 0) return;

    const int aFwooshRemaining = TodAnimateCurve(50, 0, --mBoardData.mFwooshCountDown, 12, 0, TodCurves::CURVE_LINEAR);
    for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++) {
        for (int i = 0; i < 12 - aFwooshRemaining; i++) {
            Reanimation *aFwoosh = mApp->ReanimationTryToGet(mBoardData.mFwooshID[aRow][i]);
            if (aFwoosh) {
                aFwoosh->SetFramesForLayer("anim_done");
                aFwoosh->mAnimRate = 15;
                aFwoosh->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME;
            }
            mBoardData.mFwooshID[aRow][i] = ReanimationID::REANIMATIONID_NULL;
        }
    }
}

// 0x41D730
void Board::UpdateGridItems() {
    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (mBoardData.mEnableGraveStones && aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE &&
            aGridItem->mGridItemCounter < 100) {
            aGridItem->mGridItemCounter++;
        }

        if (aGridItem->mGridItemType == GridItemType::GRIDITEM_CRATER &&
            mApp->mGameScene == GameScenes::SCENE_PLAYING) {
            if (aGridItem->mGridItemCounter > 0) {
                aGridItem->mGridItemCounter--;
            }
            if (aGridItem->mGridItemCounter == 0) {
                aGridItem->GridItemDie();
            }
        }
        aGridItem->Update();
    }
}

// 0x41D7D0
bool Board::PlantingRequirementsMet(const SeedType theSeedType) {
    switch (theSeedType) {
    case SeedType::SEED_GATLINGPEA:    return CountPlantByType(SeedType::SEED_REPEATER);
    case SeedType::SEED_TWINSUNFLOWER: return CountPlantByType(SeedType::SEED_SUNFLOWER);
    case SeedType::SEED_GLOOMSHROOM:   return CountPlantByType(SeedType::SEED_FUMESHROOM);
    case SeedType::SEED_CATTAIL:       return CountEmptyPotsOrLilies(SeedType::SEED_LILYPAD);
    case SeedType::SEED_WINTERMELON:   return CountPlantByType(SeedType::SEED_MELONPULT);
    case SeedType::SEED_GOLD_MAGNET:   return CountPlantByType(SeedType::SEED_MAGNETSHROOM);
    case SeedType::SEED_SPIKEROCK:     return CountPlantByType(SeedType::SEED_SPIKEWEED);
    case SeedType::SEED_COBCANNON:     return HasValidCobCannonSpot();
    default:                           return true;
    }
}

// 0x41D8A0
//  GOTY @Patoke: 0x420670
int Board::KillAllZombiesInRadius(
    const int theRow, const int theX, const int theY, const int theRadius, const int theRowRange, const bool theBurn,
    const int theDamageRangeFlags
) {
    Zombie *aZombie = nullptr;
    int aKilledZombies = 0; // @Patoke: implemented this
    while (IterateZombies(aZombie)) {
        if (aZombie->EffectedByDamage(theDamageRangeFlags)) {
            Rect aZombieRect = aZombie->GetZombieRect();
            int aRowDist = aZombie->mRow - theRow;
            if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) {
                aRowDist = 0;
            }

            if (aRowDist <= theRowRange && aRowDist >= -theRowRange &&
                GetCircleRectOverlap(theX, theY, theRadius, aZombieRect)) {
                if (theBurn) {
                    aZombie->ApplyBurn();
                } else {
                    aZombie->TakeDamage(1800, 18U);
                }

                aKilledZombies++;
            }
        }
    }

    const int aGridX = PixelToGridXKeepOnBoard(theX, theY);
    const int aGridY = PixelToGridYKeepOnBoard(theX, theY);
    GridItem *aGridItem = nullptr;
    while (IterateGridItems(aGridItem)) {
        if (aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER) {
            if (GridInRange(aGridItem->mGridX, aGridItem->mGridY, aGridX, aGridY, theRowRange, theRowRange)) {
                aGridItem->GridItemDie();
            }
        }
    }

    return aKilledZombies;
}

// 0x41DA10
int Board::GetNumWavesPerSurvivalStage() {
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mApp->IsSurvivalNormal(mApp->mGameMode)) {
        return 10;
    } else if (mApp->IsSurvivalHard(mApp->mGameMode) || mApp->IsSurvivalEndless(mApp->mGameMode)) {
        return 20;
    }

    TOD_ASSERT();

    unreachable();
}

// 0x41DA50
void Board::RemoveParticleByType(const ParticleEffect theEffectType) {
    TodParticleSystem *aParticle = nullptr;
    while (IterateParticles(aParticle)) {
        if (aParticle->mEffectType == theEffectType) {
            aParticle->ParticleSystemDie();
        }
    }
}

// 0x41DA90
bool Board::PlantUsesAcceleratedPricing(const SeedType theSeedType) {
    return Plant::IsUpgrade(theSeedType) && mApp->IsSurvivalEndless(mApp->mGameMode);
}

// 0x41DAE0
int Board::GetCurrentPlantCost(const SeedType theSeedType, const SeedType theImitaterType) {
    int aCost = Plant::GetCost(theSeedType, theImitaterType);
    if (PlantUsesAcceleratedPricing(theSeedType)) {
        aCost += CountPlantByType(theSeedType) * 50;
    }
    return aCost;
}

// 0x41DB40
bool Board::CanUseGameObject(const GameObjectType theGameObject) {
    if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) {
        return (theGameObject == GameObjectType::OBJECT_TYPE_TREE_FOOD) ||
               (theGameObject == GameObjectType::OBJECT_TYPE_NEXT_GARDEN);
    }
    if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) {
        return false;
    }

    if (theGameObject == GameObjectType::OBJECT_TYPE_WATERING_CAN) {
        return true;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_NEXT_GARDEN) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_MUSHROOM_GARDEN)] ||
               mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_AQUARIUM_GARDEN)] ||
               mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_TREE_OF_WISDOM)];
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_FERTILIZER) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_FERTILIZER)] > 0;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_BUG_SPRAY) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_BUG_SPRAY)] > 0;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_PHONOGRAPH) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_PHONOGRAPH)] > 0;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_CHOCOLATE) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_CHOCOLATE)] > 0;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_WHEELBARROW) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_WHEEL_BARROW)] > 0;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_GLOVE) {
        return mApp->mPlayerInfo->mPurchases[static_cast<int>(StoreItem::STORE_ITEM_GARDENING_GLOVE)] > 0;
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_MONEY_SIGN) {
        return mApp->HasFinishedAdventure();
    }
    if (theGameObject == GameObjectType::OBJECT_TYPE_TREE_FOOD) {
        return false;
    }

    TOD_ASSERT();

    unreachable();
}

void Board::ShakeBoard(const int theShakeAmountX, const int theShakeAmountY) {
    mBoardData.mShakeCounter = 12;
    mBoardData.mShakeAmountX = theShakeAmountX;
    mBoardData.mShakeAmountY = theShakeAmountY;
}

LawnMower *Board::FindLawnMowerInRow(const int theRow) {
    LawnMower *aLawnMower = nullptr;
    while (IterateLawnMowers(aLawnMower)) {
        if (aLawnMower->mRow == theRow) {
            return aLawnMower;
        }
    }
    return nullptr;
}

Zombie *Board::GetWinningZombie() {
    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mFromWave == Zombie::ZOMBIE_WAVE_WINNER) {
            return aZombie;
        }
    }
    return nullptr;
}

int Board::CountZombieByType(const ZombieType theZombieType) {
    int aCount = 0;

    Zombie *aZombie = nullptr;
    while (IterateZombies(aZombie)) {
        if (aZombie->mZombieType == theZombieType) {
            aCount++;
        }
    }

    return aCount;
}

int Board::NumberZombiesInWave(const int theWaveIndex) {
    TOD_ASSERT(theWaveIndex >= 0 && theWaveIndex < MAX_ZOMBIE_WAVES && theWaveIndex < mBoardData.mNumWaves);

    for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++) {
        if (mBoardData.mZombiesInWave[theWaveIndex][i] == ZombieType::ZOMBIE_INVALID) {
            return i;
        }
    }

    TOD_ASSERT();
    return 0;
}

bool Board::IsZombieTypeSpawnedOnly(const ZombieType theZombieType) {
    return (
        theZombieType == ZombieType::ZOMBIE_BACKUP_DANCER || theZombieType == ZombieType::ZOMBIE_BOBSLED ||
        theZombieType == ZombieType::ZOMBIE_IMP
    );
}

bool &Board::IsHelpDisplayed(AdviceType theHelpIndex) {
    TOD_ASSERT(theHelpIndex > AdviceType::ADVICE_NONE && theHelpIndex < AdviceType::NUM_ADVICE_TYPES);
    return mBoardData.mHelpDisplayed[static_cast<int>(theHelpIndex)];
}
