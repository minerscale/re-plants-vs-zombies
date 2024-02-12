#include "PoolEffect.h"
#include "GameConstants.h"
#include "LawnApp.h"
#include "Resources.h"
#include "graphics/VkCommon.h"
#include "todlib/TodDebug.h"
// #include "graphics/DDImage.h"
#include "Common.h"
#include "graphics/Graphics.h"
// #include "graphics/MemoryImage.h"
#include <cmath>
#include <cstdio>
// #include "graphics/DDInterface.h"
// #include "graphics/D3DInterface.h"

// 0x469A60
void PoolEffect::PoolEffectInitialize() {
    TodHesitationBracket aHesitation("PoolEffectInitialize");

    mApp = gLawnApp;

    static bool has_shown = false;
    if (!has_shown) printf("warning:  PoolEffect totally doesn't exist lol\n");
    has_shown = true;

    Sexy::ResourceManager::ImageRes aRes;
    aRes.mPath = "./images/pool_caustic_effect.jpg";
    mCausticGrayscaleImage = ImageLib::GetImage(aRes, false);

    mCausticImage = std::make_unique<Vk::VkImage>(CAUSTIC_IMAGE_WIDTH, CAUSTIC_IMAGE_HEIGHT, false, true);

    Vk::createBuffer(
        CAUSTIC_SIZE_BYTES, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mStagingBuffer, mStagingBufferMemory
    );
}

void PoolEffect::PoolEffectDispose() {
    Vk::doDeleteInfo({
        {},
        {},
        {},
        mStagingBufferMemory,
        {},
        mStagingBuffer,
    });
    // unreachable();
    // delete mCausticImage;
    // delete[] mCausticGrayscaleImage;
}

// 0x469BC0
unsigned int PoolEffect::BilinearLookupFixedPoint(unsigned int u, unsigned int v) const {
    const unsigned int timeU = u & 0xFFFF0000;
    const unsigned int timeV = v & 0xFFFF0000;
    const unsigned int factorU1 = ((u - timeU) & 0x0000FFFE) + 1;
    const unsigned int factorV1 = ((v - timeV) & 0x0000FFFE) + 1;
    const unsigned int factorU0 = 65536 - factorU1;
    const unsigned int factorV0 = 65536 - factorV1;
    const unsigned int indexU0 = (timeU >> 16) % 256;
    const unsigned int indexU1 = ((timeU >> 16) + 1) % 256;
    const unsigned int indexV0 = (timeV >> 16) % 256;
    const unsigned int indexV1 = ((timeV >> 16) + 1) % 256;

    const uint32_t *aBits = mCausticGrayscaleImage->mBits.get();
    return ((((factorU0 * factorV1) >> 16) * (aBits[indexV1 * 256 + indexU0] & 0xFF000000 >> 24)) >> 16) +
           ((((factorU1 * factorV1) >> 16) * (aBits[indexV1 * 256 + indexU1] & 0xFF000000 >> 24)) >> 16) +
           ((((factorU0 * factorV0) >> 16) * (aBits[indexV0 * 256 + indexU0] & 0xFF000000 >> 24)) >> 16) +
           ((((factorU1 * factorV0) >> 16) * (aBits[indexV0 * 256 + indexU1] & 0xFF000000 >> 24)) >> 16);
}

// 0x469CA0
void PoolEffect::UpdateWaterEffect() {
    // static bool has_shown = false;
    // if(!has_shown) printf("TODO:    write compute shader for updating the water effect.\n");
    // has_shown = true;

    uint32_t *data;
    vkMapMemory(Vk::device, mStagingBufferMemory, 0, CAUSTIC_SIZE_BYTES, 0, (void **)&data);

    int idx = 0;
    for (int y = 0; y < CAUSTIC_IMAGE_HEIGHT * SCALE; y++) {
        const int timeV1 = (256 - y) << 17;
        const int timeV0 = y << 17;

        for (int x = 0; x < CAUSTIC_IMAGE_WIDTH * SCALE; x++) {
            uint32_t *pix = &data[idx];

            const int timeU = x << 17;
            const int timePool0 = mPoolCounter << 16;
            const int timePool1 = ((mPoolCounter & 65535) + 1) << 16;
            const int a1 = static_cast<unsigned char>(
                BilinearLookupFixedPoint((timeU - timePool1 / 6) / SCALE, (timeV1 + timePool0 / 8) / SCALE)
            );
            const int a0 =
                static_cast<unsigned char>(BilinearLookupFixedPoint((timeU + timePool0 / 10) / SCALE, timeV0 / SCALE));
            const unsigned char a = static_cast<unsigned char>((a0 + a1) / 2);

            unsigned char alpha;
            if (a >= 160U) {
                alpha = 255 - 2 * (a - 160U);
            } else if (a >= 128U) {
                alpha = 5 * (a - 128U);
            } else {
                alpha = 0;
            }

            const uint8_t v = alpha / 3;
            // data is premultiplied so all values are the same.
            *pix = v | v << 8 | v << 16 | v << 24;
            idx++;
        }
    }

    vkUnmapMemory(Vk::device, mStagingBufferMemory);

    Vk::renderMutex.lock();
    mCausticImage->uploadNewData(mStagingBuffer);
    Vk::renderMutex.unlock();
}

// 0x469DE0
void PoolEffect::PoolEffectDraw(Sexy::Graphics *g, bool theIsNight) {
    /*
    if (!mApp->Is3DAccelerated())
    {
        if (theIsNight)
        {
            g->DrawImage(IMAGE_POOL_NIGHT, 34, 278);
        }
        else
        {
            g->DrawImage(IMAGE_POOL, 34, 278);
        }
        return;
    }*/

    const float aGridSquareX = IMAGE_POOL->GetWidth() / 15.0f;
    const float aGridSquareY = IMAGE_POOL->GetHeight() / 5.0f;
    float aOffsetArray[3][16][6][2] = {{{{0}}}};
    for (int x = 0; x <= 15; x++) {
        for (int y = 0; y <= 5; y++) {
            aOffsetArray[2][x][y][0] = x / 15.0f;
            aOffsetArray[2][x][y][1] = y / 5.0f;
            if (x != 0 && x != 15 && y != 0 && y != 5) {
                const float aPoolPhase = mPoolCounter * 2 * PI;
                const float aWaveTime1 = aPoolPhase / 800.0;
                const float aWaveTime2 = aPoolPhase / 150.0;
                const float aWaveTime3 = aPoolPhase / 900.0;
                const float aWaveTime4 = aPoolPhase / 800.0;
                const float aWaveTime5 = aPoolPhase / 110.0;
                const float xPhase = x * 3.0f * 2 * PI / 15.0f;
                const float yPhase = y * 3.0f * 2 * PI / 5.0f;

                aOffsetArray[0][x][y][0] = sin(yPhase + aWaveTime2) * 0.002f + sin(yPhase + aWaveTime1) * 0.005f;
                aOffsetArray[0][x][y][1] = sin(xPhase + aWaveTime5) * 0.01f + sin(xPhase + aWaveTime3) * 0.015f +
                                           sin(xPhase + aWaveTime4) * 0.005f;
                aOffsetArray[1][x][y][0] =
                    sin(yPhase * 0.2f + aWaveTime2) * 0.015f + sin(yPhase * 0.2f + aWaveTime1) * 0.012f;
                aOffsetArray[1][x][y][1] = sin(xPhase * 0.2f + aWaveTime5) * 0.005f +
                                           sin(xPhase * 0.2f + aWaveTime3) * 0.015f +
                                           sin(xPhase * 0.2f + aWaveTime4) * 0.02f;
                aOffsetArray[2][x][y][0] +=
                    sin(yPhase + aWaveTime1 * 1.5f) * 0.004f + sin(yPhase + aWaveTime2 * 1.5f) * 0.005f;
                aOffsetArray[2][x][y][1] += sin(xPhase * 4.0f + aWaveTime5 * 2.5f) * 0.005f +
                                            sin(xPhase * 2.0f + aWaveTime3 * 2.5f) * 0.04f +
                                            sin(xPhase * 3.0f + aWaveTime4 * 2.5f) * 0.02f;
            } else {
                aOffsetArray[0][x][y][0] = 0.0f;
                aOffsetArray[0][x][y][1] = 0.0f;
                aOffsetArray[1][x][y][0] = 0.0f;
                aOffsetArray[1][x][y][1] = 0.0f;
            }
        }
    }

    const int aIndexOffsetX[6] = {0, 0, 1, 0, 1, 1};
    const int aIndexOffsetY[6] = {0, 1, 1, 0, 1, 0};
    // TriVertex aVertArray[3][150][3];

    std::array<std::array<std::array<TriVertex, 3>, 150>, 3> aVertArray;

    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 5; y++) {
            for (int aLayer = 0; aLayer < 3; aLayer++) {
                TriVertex *pVert = &aVertArray[aLayer][x * 10 + y * 2][0];
                for (int aVertIndex = 0; aVertIndex < 6; aVertIndex++, pVert++) {
                    const int aIndexX = x + aIndexOffsetX[aVertIndex];
                    const int aIndexY = y + aIndexOffsetY[aVertIndex];
                    if (aLayer == 2) {
                        pVert->x = (704.0f / 15.0f) * aIndexX + 45.0f;
                        pVert->y = 30.0f * aIndexY + 288.0f;
                        pVert->u = aOffsetArray[2][aIndexX][aIndexY][0] + aIndexX / 15.0f;
                        pVert->v = aOffsetArray[2][aIndexX][aIndexY][1] + aIndexY / 5.0f;

                        if (!g->mClipRect.Contains(pVert->x, pVert->y)) {
                            pVert->color = 0x00FFFFFFUL;
                        } else if (aIndexX == 0 || aIndexX == 15 || aIndexY == 0) {
                            pVert->color = 0x20FFFFFFUL;
                        } else if (theIsNight) {
                            pVert->color = 0x30FFFFFFUL;
                        } else {
                            pVert->color = aIndexX <= 7 ? 0xC0FFFFFFUL : 0x80FFFFFFUL;
                        }
                    } else {
                        pVert->color = 0xFFFFFFFFUL;
                        pVert->x = aIndexX * aGridSquareX + 35.0f;
                        pVert->y = aIndexY * aGridSquareY + 279.0f;
                        pVert->u = aOffsetArray[aLayer][aIndexX][aIndexY][0] + aIndexX / 15.0f;
                        pVert->v = aOffsetArray[aLayer][aIndexX][aIndexY][1] + aIndexY / 5.0f;
                        if (!g->mClipRect.Contains(pVert->x, pVert->y)) {
                            pVert->color = 0x00FFFFFFUL;
                        }
                    }
                }
            }
        }
    }

    if (theIsNight) {
        g->DrawTrianglesTex(IMAGE_POOL_BASE_NIGHT, aVertArray[0].data(), 150);
        g->DrawTrianglesTex(IMAGE_POOL_SHADING_NIGHT, aVertArray[1].data(), 150);
    } else {
        g->DrawTrianglesTex(IMAGE_POOL_BASE, aVertArray[0].data(), 150);
        g->DrawTrianglesTex(IMAGE_POOL_SHADING, aVertArray[1].data(), 150);
    }

    UpdateWaterEffect();

    static bool has_shown = false;
    if (!has_shown) printf("TODO:    fix drawing of pool effect.\n");
    has_shown = true;
    // unreachable();
    //  FIXME
    // D3DInterface* anInterface = ((DDImage*)g->mDestImage)->mDDInterface->mD3DInterface;
    // anInterface->CheckDXError(anInterface->mD3DDevice->SetTextureStageState(0,
    // D3DTEXTURESTAGESTATETYPE::D3DTSS_ADDRESSU, D3DTEXTUREADDRESS::D3DTADDRESS_WRAP), "DrawPool");
    // anInterface->CheckDXError(anInterface->mD3DDevice->SetTextureStageState(0,
    // D3DTEXTURESTAGESTATETYPE::D3DTSS_ADDRESSV, D3DTEXTUREADDRESS::D3DTADDRESS_WRAP), "DrawPool");
    g->DrawTrianglesTex(mCausticImage.get(), aVertArray[2].data(), 150);
    // anInterface->CheckDXError(anInterface->mD3DDevice->SetTextureStageState(0,
    // D3DTEXTURESTAGESTATETYPE::D3DTSS_ADDRESSU, D3DTEXTUREADDRESS::D3DTADDRESS_CLAMP), "DrawPool");
    // anInterface->CheckDXError(anInterface->mD3DDevice->SetTextureStageState(0,
    // D3DTEXTURESTAGESTATETYPE::D3DTSS_ADDRESSV, D3DTEXTUREADDRESS::D3DTADDRESS_CLAMP), "DrawPool");
}

void PoolEffect::PoolEffectUpdate() { ++mPoolCounter; }
