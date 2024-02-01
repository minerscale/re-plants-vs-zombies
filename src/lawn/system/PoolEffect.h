#ifndef __POOLEFFECT_H__
#define __POOLEFFECT_H__

#include "framework/graphics/Image.h"
#include "graphics/VkCommon.h"
#include "graphics/VkImage.h"
#include <array>
#include <memory>

constexpr const int CAUSTIC_IMAGE_WIDTH = 128;
constexpr const int CAUSTIC_IMAGE_HEIGHT = 64;
constexpr const size_t CAUSTIC_SIZE_BYTES =
    CAUSTIC_IMAGE_WIDTH * CAUSTIC_IMAGE_HEIGHT * SCALE * SCALE * sizeof(uint32_t);

class LawnApp;
class PoolEffect {
public:
    std::unique_ptr<ImageLib::Image> mCausticGrayscaleImage;
    std::array<std::array<uint32_t, CAUSTIC_IMAGE_WIDTH>, CAUSTIC_IMAGE_HEIGHT> mMemCausticImage;
    std::unique_ptr<Vk::VkImage> mCausticImage;

    VkBuffer mStagingBuffer;
    VkDeviceMemory mStagingBufferMemory;

    LawnApp *mApp;
    int mPoolCounter;

public:
    void PoolEffectInitialize();
    void PoolEffectDispose();
    void PoolEffectDraw(Sexy::Graphics *g, bool theIsNight);
    void UpdateWaterEffect();
    unsigned int BilinearLookupFixedPoint(unsigned int u, unsigned int v);
    // unsigned int		BilinearLookup(float u, float v);
    void PoolEffectUpdate();
};

#endif
