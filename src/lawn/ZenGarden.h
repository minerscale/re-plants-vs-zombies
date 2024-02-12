#ifndef __ZENGARDEN_H__
#define __ZENGARDEN_H__

#include "ConstEnums.h"
#include <cstdint>

#define ZEN_MAX_GRIDSIZE_X 8
#define ZEN_MAX_GRIDSIZE_Y 4

constexpr const float STINKY_SLEEP_POS_Y = 461.0f;

class LawnApp;
class Board;
class Plant;
class GridItem;
class HitResult;
class PottedPlant;

namespace Sexy {
class Graphics;
}

using namespace Sexy;

class SpecialGridPlacement {
public:
    int mPixelX;
    int mPixelY;
    int mGridX;
    int mGridY;
};

class ZenGarden {
public:
    LawnApp *mApp;          //+0x0
    Board *mBoard;          //+0x4
    GardenType mGardenType; //+0x8

public:
    ZenGarden();

    void ZenGardenInitLevel();
    /*inline*/
    void DrawPottedPlantIcon(Graphics *g, float x, float y, PottedPlant *thePottedPlant);
    void DrawPottedPlant(
        const Graphics *g, float x, float y, const PottedPlant *thePottedPlant, float theScale, bool theDrawPot
    );
    bool IsZenGardenFull(bool theIncludeDroppedPresents) const;
    void FindOpenZenGardenSpot(int &theSpotX, int &theSpotY) const;
    void AddPottedPlant(const PottedPlant *thePottedPlant);
    void MouseDownWithTool(int x, int y, CursorType theCursorType);
    void MovePlant(Plant *thePlant, int theGridX, int theGridY);
    void MouseDownWithMoneySign(Plant *thePlant) const;
    Plant *PlacePottedPlant(intptr_t thePottedPlantIndex);
    static float PlantPottedDrawHeightOffset(SeedType theSeedType, float theScale);
    static float ZenPlantOffsetX(const PottedPlant *thePottedPlant);
    int GetPlantSellPrice(const Plant *thePlant) const;
    void ZenGardenUpdate();
    void MouseDownWithFullWheelBarrow(int x, int y);
    void MouseDownWithEmptyWheelBarrow(Plant *thePlant);
    void GotoNextGarden();
    /*inline*/
    PottedPlant *GetPottedPlantInWheelbarrow();
    void RemovePottedPlant(Plant *thePlant) const;
    SpecialGridPlacement *GetSpecialGridPlacements(int &theCount) const;
    int PixelToGridX(int theX, int theY) const;
    int PixelToGridY(int theX, int theY) const;
    int GridToPixelX(int theGridX, int theGridY);
    int GridToPixelY(int theGridX, int theGridY);
    void DrawBackdrop(Graphics *g);
    bool MouseDownZenGarden(int x, int y, int theClickCount, const HitResult *theHitResult);
    void PlantFulfillNeed(const Plant *thePlant) const;
    void PlantWatered(const Plant *thePlant) const;
    static PottedPlantNeed GetPlantsNeed(PottedPlant *thePottedPlant);
    void MouseDownWithFeedingTool(int x, int y, CursorType theCursorType);
    void DrawPlantOverlay(Graphics *g, const Plant *thePlant);
    PottedPlant *PottedPlantFromIndex(intptr_t thePottedPlantIndex) const;
    static bool WasPlantNeedFulfilledToday(const PottedPlant *thePottedPlant);
    void PottedPlantUpdate(Plant *thePlant);
    void AddHappyEffect(Plant *thePlant) const;
    void RemoveHappyEffect(const Plant *thePlant) const;
    void PlantUpdateProduction(Plant *thePlant);
    bool CanDropPottedPlantLoot() const;
    void ShowTutorialArrowOnWateringCan();
    bool PlantsNeedWater() const;
    static void ZenGardenStart();
    void UpdatePlantEffectState(Plant *thePlant);
    bool CanUseGameObject(GameObjectType);
    void ZenToolUpdate(GridItem *theZenTool);
    void DoFeedingTool(int x, int y, GridItemState theToolType);
    void AddStinky();
    void StinkyUpdate(GridItem *theStinky);
    void OpenStore();
    GridItem *GetStinky();
    void StinkyPickGoal(GridItem *theStinky);
    static bool PlantShouldRefreshNeed(const PottedPlant *thePottedPlant);
    void PlantFertilized(Plant *thePlant);
    static bool WasPlantFertilizedInLastHour(const PottedPlant *thePottedPlant);
    void SetupForZenTutorial();
    bool HasPurchasedStinky();
    int CountPlantsNeedingFertilizer() const;
    bool AllPlantsHaveBeenFertilized() const;
    void WakeStinky();
    bool ShouldStinkyBeAwake();
    bool IsStinkySleeping();
    static SeedType PickRandomSeedType();
    void StinkyWakeUp(GridItem *theStinky);
    void StinkyStartFallingAsleep(GridItem *theStinky);
    void StinkyFinishFallingAsleep(GridItem *theStinky, int theBlendTime);
    void AdvanceCrazyDaveDialog();
    void LeaveGarden();
    bool CanDropChocolate();
    void FeedChocolateToPlant(Plant *thePlant);
    bool PlantHighOnChocolate(const PottedPlant *thePottedPlant);
    bool PlantCanHaveChocolate(const Plant *thePlant);
    void SetPlantAnimSpeed(const Plant *thePlant);
    void UpdateStinkyMotionTrail(GridItem *theStinky, bool theStinkyHighOnChocolate);
    void ResetPlantTimers(PottedPlant *thePottedPlant);
    void ResetStinkyTimers();
    void UpdatePlantNeeds();
    static void RefreshPlantNeeds(PottedPlant *thePottedPlant);
    void PlantSetLaunchCounter(Plant *thePlant);
    int PlantGetMinutesSinceHappy(const Plant *thePlant);
    /*inline*/
    bool IsStinkyHighOnChocolate();
    void StinkyAnimRateUpdate(const GridItem *theStinky);
    /*inline*/
    bool PlantCanBeWatered(const Plant *thePlant) const;
};

#endif
