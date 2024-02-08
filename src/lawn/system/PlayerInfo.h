#ifndef __PLAYERINFO_H__
#define __PLAYERINFO_H__

#define MAX_POTTED_PLANTS 200
#define PURCHASE_COUNT_OFFSET 1000

#include "ConstEnums.h"
#ifdef __GNUC__
#include <bits/chrono.h>
#else
#include <chrono>
#endif

class TimePoint : public std::chrono::time_point<std::chrono::system_clock> {
public:
    TimePoint() : std::chrono::time_point<std::chrono::system_clock>() {}
    TimePoint(const std::chrono::time_point<std::chrono::system_clock> &t)
        : std::chrono::time_point<std::chrono::system_clock>(t) {}
    TimePoint(const std::chrono::seconds &t) : std::chrono::time_point<std::chrono::system_clock>(t) {}
    TimePoint(const std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<int64_t, std::nano>> &t) {
        *this = std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch())
        );
    }
};

class PottedPlant {
public:
    enum FacingDirection { FACING_RIGHT, FACING_LEFT };

public:
    SeedType mSeedType;         //+0x0
    GardenType mWhichZenGarden; //+0x4
    int mX;                     //+0x8
    int mY;                     //+0xC
    FacingDirection mFacing;    //+0x10

    uint64_t mLastWateredTime;    //+0x18
    DrawVariation mDrawVariation; //+0x20
    PottedPlantAge mPlantAge;     //+0x24
    int mTimesFed;                //+0x28
    int mFeedingsPerGrow;         //+0x2C
    PottedPlantNeed mPlantNeed;   //+0x30

    uint64_t mLastNeedFulfilledTime; //+0x38
    uint64_t mLastFertilizedTime;    //+0x40
    uint64_t mLastChocolateTime;     //+0x48
    uint64_t mFutureAttribute[1];    //+0x50

public:
    void InitializePottedPlant(SeedType theSeedType);
};

static inline TimePoint getTime() { return std::chrono::system_clock::now(); }

static inline uint32_t TimeToUnixEpoch(const TimePoint &t) {
    return std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
}

static inline TimePoint TimeFromUnixEpoch(const uint32_t &t) { return TimePoint(std::chrono::seconds(t)); }

class DataSync;
class PlayerInfo {
public:
    SexyString mName;                                                     //+0x0
    uint32_t mUseSeq;                                                     //+0x1C
    uint32_t mId;                                                         //+0x20
    int mLevel;                                                           //+0x24
    int mCoins;                                                           //+0x28
    int mFinishedAdventure;                                               //+0x2C
    int mChallengeRecords[100];                                           //+0x30
    int32_t mPurchases[80];                                               //+0x1C0
    std::chrono::high_resolution_clock::duration mPlayTimeActivePlayer;   //+0x300
    std::chrono::high_resolution_clock::duration mPlayTimeInactivePlayer; //+0x304
    int mHasUsedCheatKeys;                                                //+0x308
    int mHasWokenStinky;                                                  //+0x30C
    int mDidntPurchasePacketUpgrade;                                      //+0x310
    TimePoint mLastStinkyChocolateTime;                                   //+0x314
    int mStinkyPosX;                                                      //+0x318
    int mStinkyPosY;                                                      //+0x31C
    int mHasUnlockedMinigames;                                            //+0x320
    int mHasUnlockedPuzzleMode;                                           //+0x324
    int mHasNewMiniGame;                                                  //+0x328
    int mHasNewScaryPotter;                                               //+0x32C
    int mHasNewIZombie;                                                   //+0x330
    int mHasNewSurvival;                                                  //+0x334
    int mHasUnlockedSurvivalMode;                                         //+0x338
    int mNeedsMessageOnGameSelector;                                      //+0x33C
    int mNeedsMagicTacoReward;                                            //+0x340
    int mHasSeenStinky;                                                   //+0x344
    int mHasSeenUpsell;                                                   //+0x348
    int mPlaceHolderPlayerStats;                                          //+0x??????
    int mNumPottedPlants;                                                 //+0x350
    PottedPlant mPottedPlant[MAX_POTTED_PLANTS];                          //+0x358
    bool mEarnedAchievements[20];                                         //+GOTY @Patoke: 0x24
    bool mShownAchievements[20];                                          //+GOTY @Patoke: 0x38

public:
    PlayerInfo();

    void Reset();
    /*inline*/ void AddCoins(int theAmount);
    void SyncSummary(DataSync &theSync);
    void SyncDetails(DataSync &theSync);
    void DeleteUserFiles();
    void LoadDetails();
    void SaveDetails();
    inline int GetLevel() const { return mLevel; }
    inline void SetLevel(int theLevel) { mLevel = theLevel; }
    /*inline*/ void ResetChallengeRecord(GameMode theGameMode);
};

#endif
