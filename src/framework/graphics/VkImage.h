#ifndef __VK_IMAGE_H__
#define __VK_IMAGE_H__

#include <cstdio>
#include <memory>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Image.h"
#include "imagelib/ImageLib.h"
#include "todlib/FilterEffect.h"

using namespace Sexy;

namespace Vk {
class VkImage : public Image {
public:
    VkImage(const ImageLib::Image &theImage);
    VkImage(int width, int height, bool initialise = true, bool textureRepeat = false);
    VkImage(const Image &theImage) = delete;
    VkImage(VkImage &theImage);
    VkImage &operator=(const Image &) = delete;
    ~VkImage() override;

    bool mHasTrans = false; // unused, but set by other parts of the code. Feel free to deprecate this.
    bool mHasAlpha = false; // unused, but set by other parts of the code. Feel free to deprecate this.

    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    ::VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkDescriptorSet descriptor = VK_NULL_HANDLE;

    void TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

    std::unique_ptr<VkImage> applyEffectsToNewImage(FilterEffect theFilterEffect);
    static void applyEffects(VkImage *theSrcImage, VkImage *theDestImage, FilterEffect theFilterEffect);
    void applyEffectsToSelf(FilterEffect theFilterEffect);
    void uploadNewData(VkBuffer stagingBuffer);

    bool PolyFill3D(
        const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode,
        int tx, int ty
    ) override;
    void FillRect(const Rect &theRect, const Color &theColor, int theDrawMode) override;
    void ClearRect(const Rect &theRect) override;
    void DrawLine(
        double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor, int theDrawMode
    ) override;
    void DrawLineAA(
        double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor, int theDrawMode
    ) override;
    void FillScanLinesWithCoverage(
        Span *theSpans, int theSpanCount, const Color &theColor, int theDrawMode, const unsigned char *theCoverage,
        int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight
    ) override;
    void
    Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode) override;
    void BltF(
        Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor,
        int theDrawMode
    ) override;
    void BltRotated(
        Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor,
        int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY
    ) override;
    void StretchBlt(
        Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect,
        const Color &theColor, int theDrawMode, bool fastStretch
    ) override;
    void BltMatrix(
        Image *theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect &theClipRect, const Color &theColor,
        int theDrawMode, const Rect &theSrcRect, bool blend
    ) override;
    void BltTrianglesTex(
        Image *theTexture, const std::array<TriVertex, 3> *theVertices, int theNumTriangles, const Rect &theClipRect,
        const Color &theColor, int theDrawMode, float tx, float ty, bool blend
    ) override;
    void BltMirror(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode)
        override;
    void StretchBltMirror(
        Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect,
        const Color &theColor, int theDrawMode, bool fastStretch
    ) override;

private:
    void BltEx(
        Image *theImage, const std::array<glm::vec4, 4> &vertices, const glm::vec4 &theClipRect, const Color &theColor,
        const int theDrawMode, bool blend
    );
    void BltMatrixHelper(
        Image *theImage, const glm::mat3 &theMatrix, const glm::vec4 &theSrcRect, const glm::vec4 &theClipRect,
        const Color &theColor, int theDrawMode, bool blend
    );
    void BeginDraw(Image *theImage, int theDrawMode);
    void SetViewportAndScissor(const glm::vec4 &theClipRect) const;
};
} // namespace Vk

#endif // __VK_IMAGE_H__
