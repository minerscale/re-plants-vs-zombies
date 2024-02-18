#include "PlayerInfo.h"
#include "DataSync.h"
#include "framework/SexyAppBase.h"
#include "lawn/LawnCommon.h"
#include "lawn/widget/ChallengeScreen.h"
#include "misc/Buffer.h"
#include "todlib/TodCommon.h"
#include "todlib/TodDebug.h"
#include <chrono>
#include <ratio>

static int gUserVersion = 12;

PlayerInfo::PlayerInfo() { Reset(); }

// 0x468310
void PlayerInfo::SyncSummary(DataSync &theSync) {
    theSync.SyncString(mName);
    theSync.SyncLong(mUseSeq);
    theSync.SyncLong(mId);
}

// 0x468390
void PlayerInfo::SyncDetails(DataSync &theSync) {
    if (theSync.GetReader()) {
        Reset();
    }

    int aVersion = gUserVersion;
    theSync.SyncLong(aVersion);
    theSync.SetVersion(aVersion);
    if (aVersion != gUserVersion) {
        return;
    }

    theSync.SyncLong(mLevel);
    theSync.SyncLong(mCoins);
    theSync.SyncLong(mFinishedAdventure);
    for (int i = 0; i < 100; i++) {
        theSync.SyncLong(mChallengeRecords[i]);
    }
    for (int i = 0; i < 80; i++) {
        theSync.SyncBytes(&mPurchases[i], sizeof(mPurchases[i]));
    }
    theSync.SyncTime<std::chrono::milliseconds>(mPlayTimeActivePlayer);
    theSync.SyncTime<std::chrono::milliseconds>(mPlayTimeInactivePlayer);
    theSync.SyncLong(mHasUsedCheatKeys);
    theSync.SyncLong(mHasWokenStinky);
    theSync.SyncLong(mDidntPurchasePacketUpgrade);
    theSync.SyncTime<std::chrono::seconds>(mLastStinkyChocolateTime);
    theSync.SyncLong(mStinkyPosX);
    theSync.SyncLong(mStinkyPosY);
    theSync.SyncLong(mHasUnlockedMinigames);
    theSync.SyncLong(mHasUnlockedPuzzleMode);
    theSync.SyncLong(mHasNewMiniGame);
    theSync.SyncLong(mHasNewScaryPotter);
    theSync.SyncLong(mHasNewIZombie);
    theSync.SyncLong(mHasNewSurvival);
    theSync.SyncLong(mHasUnlockedSurvivalMode);
    theSync.SyncLong(mNeedsMessageOnGameSelector);
    theSync.SyncLong(mNeedsMagicTacoReward);
    theSync.SyncLong(mHasSeenStinky);
    theSync.SyncLong(mHasSeenUpsell);
    theSync.SyncLong(mPlaceHolderPlayerStats);
    theSync.SyncLong(mNumPottedPlants);

    TOD_ASSERT(mNumPottedPlants <= MAX_POTTED_PLANTS);
    for (int i = 0; i < mNumPottedPlants; i++) {
        theSync.SyncBytes(&mPottedPlant[i], sizeof(PottedPlant));
    }

    // @Patoke: implemented
    for (int i = 0; i < 20; i++) {
        theSync.SyncBool(mEarnedAchievements[i]);
    }

    for (int i = 0; i < 20; i++) {
        theSync.SyncBool(mShownAchievements[i]);
    }
}

// 0x469400
void PlayerInfo::LoadDetails() {
    try {
        Buffer aBuffer;
        const std::string aFileName = GetAppDataFolder() + fmt::format("userdata/user{}.dat", mId);
        if (!gSexyAppBase->ReadBufferFromFile(aFileName, &aBuffer, false)) {
            return;
        }

        DataReader aReader;
        aReader.OpenMemory(aBuffer.GetDataPtr(), aBuffer.GetDataLen(), false);
        DataSync aSync(aReader);
        SyncDetails(aSync);
    } catch (DataReaderException &) {
        fmt::println("Failed to player data, resetting it\n");
        Reset();
    }
}

// 0x4695F0
//  GOTY @Patoke: 0x46D750
void PlayerInfo::SaveDetails() {
    DataWriter aWriter;
    aWriter.OpenMemory();
    DataSync aSync(aWriter);
    SyncDetails(aSync);

    MkDir(GetAppDataFolder() + "userdata");
    const std::string aFileName = GetAppDataFolder() + fmt::format("userdata/user{}.dat", mId);
    gSexyAppBase->WriteBytesToFile(aFileName, aWriter.GetDataPtr(), aWriter.GetDataLen());
}

// 0x469810
void PlayerInfo::DeleteUserFiles() const {
    const std::string aFilename = GetAppDataFolder() + fmt::format("userdata/user{}.dat", mId);
    gSexyAppBase->EraseFile(aFilename);

    for (int i = 0; i < static_cast<int>(GameMode::NUM_GAME_MODES); i++) {
        std::string aFileName = GetSavedGameName(static_cast<GameMode>(i), mId);
        gSexyAppBase->EraseFile(aFileName);
    }
}

// 0x469940
void PlayerInfo::Reset() {
    mLevel = 1;
    mCoins = 0;
    mFinishedAdventure = 0;
    memset(mChallengeRecords, 0, sizeof(mChallengeRecords));
    memset(mPurchases, 0, sizeof(mPurchases));
    mPlayTimeActivePlayer = std::chrono::milliseconds(0);
    mPlayTimeInactivePlayer = std::chrono::milliseconds(0);
    mHasUsedCheatKeys = 0;
    mHasWokenStinky = 0;
    mDidntPurchasePacketUpgrade = 0;
    mLastStinkyChocolateTime = {};
    mStinkyPosX = 0;
    mStinkyPosY = 0;
    mHasUnlockedMinigames = 0;
    mHasUnlockedPuzzleMode = 0;
    mHasNewMiniGame = 0;
    mHasNewScaryPotter = 0;
    mHasNewIZombie = 0;
    mHasNewSurvival = 0;
    mHasUnlockedSurvivalMode = 0;
    mNeedsMessageOnGameSelector = 0;
    mNeedsMagicTacoReward = 0;
    mHasSeenStinky = 0;
    mHasSeenUpsell = 0;
    mPlaceHolderPlayerStats = 0;
    memset(mPottedPlant, 0, sizeof(mPottedPlant));
    mNumPottedPlants = 0;
    memset(mEarnedAchievements, 0, sizeof(mEarnedAchievements));
    memset(mShownAchievements, 0, sizeof(mShownAchievements));
}

void PlayerInfo::AddCoins(int theAmount) {
    mCoins += theAmount;
    if (mCoins > 99999) {
        mCoins = 99999;
    } else if (mCoins < 0) {
        mCoins = 0;
    }
}

void PlayerInfo::ResetChallengeRecord(GameMode theGameMode) {
    const int aGameMode = static_cast<int>(theGameMode) - static_cast<int>(GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_1);
    TOD_ASSERT(aGameMode >= 0 && aGameMode <= NUM_CHALLENGE_MODES);
    mChallengeRecords[aGameMode] = 0;
}

// 0x469A00
void PottedPlant::InitializePottedPlant(SeedType theSeedType) {
    memset(this, 0, sizeof(PottedPlant));
    mSeedType = theSeedType;
    mDrawVariation = DrawVariation::VARIATION_NORMAL;
    mLastWateredTime = {};
    mFacing =
        static_cast<FacingDirection>(RandRangeInt((int)FacingDirection::FACING_RIGHT, (int)FacingDirection::FACING_LEFT)
        );
    mPlantAge = PottedPlantAge::PLANTAGE_SPROUT;
    mTimesFed = 0;
    mWhichZenGarden = GardenType::GARDEN_MAIN;
    mFeedingsPerGrow = RandRangeInt(3, 5);
    mPlantNeed = PottedPlantNeed::PLANTNEED_NONE;
    mLastNeedFulfilledTime = {};
    mLastFertilizedTime = {};
    mLastChocolateTime = {};
}
