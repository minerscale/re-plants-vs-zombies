#ifndef __ALMANACDIALOG_H__
#define __ALMANACDIALOG_H__

#include "LawnDialog.h"

#define NUM_ALMANAC_SEEDS 49
#define NUM_ALMANAC_ZOMBIES 26

constexpr const float ALMANAC_PLANT_POSITION_X = 578.0f;
constexpr const float ALMANAC_PLANT_POSITION_Y = 140.0f;
constexpr const float ALMANAC_ZOMBIE_POSITION_X = 559.0f;
constexpr const float ALMANAC_ZOMBIE_POSITION_Y = 175.0f;
constexpr const int ALMANAC_INDEXPLANT_POSITION_X = 167;
constexpr const int ALMANAC_INDEXPLANT_POSITION_Y = 255;
constexpr const float ALMANAC_INDEXZOMBIE_POSITION_X = 535.0f;
constexpr const float ALMANAC_INDEXZOMBIE_POSITION_Y = 215.0f;

class Plant;
class Zombie;
class LawnApp;
class GameButton;
class Reanimation;

class AlmanacDialog : public LawnDialog {
private:
    enum { ALMANAC_BUTTON_CLOSE = 0, ALMANAC_BUTTON_PLANT = 1, ALMANAC_BUTTON_ZOMBIE = 2, ALMANAC_BUTTON_INDEX = 3 };

public:
    LawnApp *mApp;                  //+0x16C
    GameButton *mCloseButton;       //+0x170
    GameButton *mIndexButton;       //+0x174
    GameButton *mPlantButton;       //+0x178
    GameButton *mZombieButton;      //+0x17C
    AlmanacPage mOpenPage;          //+0x180
    Reanimation *mReanim[4]{};      //+0x184
    SeedType mSelectedSeed;         //+0x194
    ZombieType mSelectedZombie;     //+0x198
    Plant *mPlant;                  //+0x19C
    Zombie *mZombie;                //+0x1A0
    Zombie *mZombiePerfTest[400]{}; //+0x1A4

public:
    AlmanacDialog(LawnApp *theApp);
    ~AlmanacDialog() override;

    void ClearPlantsAndZombies();
    void RemovedFromManager(WidgetManager *theWidgetManager) override;
    void SetupPlant();
    void SetupZombie();
    void SetPage(AlmanacPage thePage);
    void Update() override;
    void DrawIndex(Graphics *g) const;
    void DrawPlants(Graphics *g) const;
    void DrawZombies(Graphics *g) const;
    void Draw(Graphics *g) override;
    static void GetSeedPosition(SeedType theSeedType, int &x, int &y);
    SeedType SeedHitTest(int x, int y) const;
    /*inline*/ bool ZombieHasSilhouette(ZombieType theZombieType) const;
    bool ZombieIsShown(ZombieType theZombieType) const;
    bool ZombieHasDescription(ZombieType theZombieType) const;
    static void GetZombiePosition(ZombieType theZombieType, int &x, int &y);
    ZombieType ZombieHitTest(int x, int y) const;
    void MouseUp(int x, int y, int theClickCount) override;
    void MouseDown(int x, int y, int theClickCount) override;
    //	virtual void				KeyChar(char theChar);

    static ZombieType GetZombieType(int theIndex);
    /*inline*/ void ShowPlant(SeedType theSeedType);
    /*inline*/ void ShowZombie(ZombieType theZombieType);
};

extern bool gZombieDefeated[NUM_ZOMBIE_TYPES];

/*inline*/ void AlmanacInitForPlayer();
/*inline*/ void AlmanacPlayerDefeatedZombie(ZombieType theZombieType);

#endif
