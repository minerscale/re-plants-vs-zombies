#ifndef __EFFECTSYSTEM_H__
#define __EFFECTSYSTEM_H__

#include "framework/graphics/Graphics.h"
#include <deque>
using namespace Sexy;

#define MAX_TRIANGLES 256

class TodTriVertex {
public:
    float x;
    float y;
    float u;
    float v;
    uint32_t color;
};

class TodTriangleGroup {
public:
    using VertexArray = std::array<std::array<TriVertex, 3>, MAX_TRIANGLES>;
    using VertexArrayPool = std::deque<VertexArray>;

    static size_t gNumVertArraysInUse;
    static VertexArrayPool gVertArrays;

    VertexArray &mVertArray;
    Image *mImage = nullptr;
    // TriVertex                   mVertArray[MAX_TRIANGLES][3];

    int mTriangleCount = 0;
    int mMaxTriangleCount = 0;
    int mDrawMode = Graphics::DRAWMODE_NORMAL;

    TodTriangleGroup();
    ~TodTriangleGroup();
    void DrawGroup(const Graphics *g);
    void AddTriangle(
        Graphics *g, Image *theImage, const SexyMatrix3 &theMatrix, const Rect &theClipRect, const Color &theColor,
        int theDrawMode, const Rect &theSrcRect
    );
};

extern bool gTodTriangleDrawAdditive; // 0x6A9EF5

class Reanimation;
class TodParticleHolder;
class TrailHolder;
class ReanimationHolder;
class AttachmentHolder;

class EffectSystem {
public:
    TodParticleHolder *mParticleHolder;
    TrailHolder *mTrailHolder;
    ReanimationHolder *mReanimationHolder;
    AttachmentHolder *mAttachmentHolder;

public:
    EffectSystem()
        : mParticleHolder(nullptr), mTrailHolder(nullptr), mReanimationHolder(nullptr), mAttachmentHolder(nullptr) {}

    ~EffectSystem() {}

    void EffectSystemInitialize();
    void EffectSystemDispose();
    void EffectSystemFreeAll() const;
    void ProcessDeleteQueue() const;
    void Update();
};

extern EffectSystem *gEffectSystem; //[0x6A9EB8]

#endif
