#include "VkImage.h"

#include "VkCommon.h"

#include "TriVertex.h"
#include "graphics/Color.h"
#include "misc/SexyMatrix.h"
#include "todlib/FilterEffect.h"
#include <tuple>

#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

#include <avir.h>
#include <avir_float4_sse.h>
#include <avir_float8_avx.h>
#include <lancir.h>

namespace Vk {
::VkImage createImage(int width, int height, VkImageUsageFlags usage) {
    static VkImageCreateInfo imageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        pixelFormat,
        {0, 0, 1}, // to be filled in later
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        0, // to be filled in later
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    imageInfo.extent.width = width * SCALE;
    imageInfo.extent.height = height * SCALE;
    imageInfo.usage = usage;

    ::VkImage ret;
    if (vkCreateImage(device, &imageInfo, nullptr, &ret) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    return ret;
}

VkDeviceMemory createImageMemory(::VkImage theImage) {
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, theImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory ret;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &ret) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, theImage, ret, 0);

    return ret;
}

VkDescriptorSet createDescriptorSet(VkImageView theView, VkSampler sampler) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet dstSet;
    vkAllocateDescriptorSets(device, &allocInfo, &dstSet);

    std::array<VkDescriptorImageInfo, 2> imageInfos{
        {{
             sampler,
             theView,
             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         }, {
             sampler,
             theView,
             VK_IMAGE_LAYOUT_GENERAL,
         }}
    };

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{
        {{
             VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
             nullptr,
             dstSet,
             0,
             0,
             1,
             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
             &imageInfos[0],
             nullptr,
             nullptr,

         }, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, dstSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          &imageInfos[1], nullptr, nullptr}}
    };

    vkUpdateDescriptorSets(device, 2, descriptorWrites.data(), 0, nullptr);

    return dstSet;
}

VkFramebuffer createFramebuffer(VkImageView theView, int theWidth, int theHeight) {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = imagePass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &theView;
    framebufferInfo.width = theWidth * SCALE;
    framebufferInfo.height = theHeight * SCALE;
    framebufferInfo.layers = 1;

    VkFramebuffer ret;
    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &ret) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }

    return ret;
}

std::atomic<int> imageBufferIdx = 0;
std::array<std::vector<deleteInfo>, NUM_IMAGE_SWAPS> deleteList;

void deferredDelete(size_t idx) {
    for (auto &i : deleteList[idx]) {
        if (i.view.has_value()) vkDestroyImageView(device, i.view.value(), nullptr);
        if (i.image.has_value()) vkDestroyImage(device, i.image.value(), nullptr);
        if (i.framebuffer.has_value()) vkDestroyFramebuffer(device, i.framebuffer.value(), nullptr);
        if (i.memory.has_value()) vkFreeMemory(device, i.memory.value(), nullptr);
        if (i.set.has_value()) vkFreeDescriptorSets(device, descriptorPool, 1, &i.set.value());
        if (i.buffer.has_value()) vkDestroyBuffer(device, i.buffer.value(), nullptr);
    }

    deleteList[idx].clear();
}

void beginCommandBuffer() {
    vkWaitForFences(device, 1, &imageFences[imageBufferIdx], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &imageFences[imageBufferIdx]);

    vkResetCommandBuffer(imageCommandBuffers[imageBufferIdx], 0);

    // Delete the oldest buffer's delete list.
    deferredDelete(imageBufferIdx);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(imageCommandBuffers[imageBufferIdx], &beginInfo);
}

bool inRenderpass = false;
int cachedDrawMode = -1;

void endRenderPass() {
    if (inRenderpass) {
        vkCmdEndRenderPass(imageCommandBuffers[imageBufferIdx]);
        inRenderpass = false;
    }
}

void doDeleteInfo(deleteInfo info) { deleteList[imageBufferIdx].emplace_back(info); }

#ifdef LANCIR_SSE2
using ImageResizerType = avir::CImageResizer<avir::fpclass_def<avir::float4>>;
#else
using ImageResizerType = avir::CImageResizer<>;
#endif
ImageResizerType ImageResizer(8);

avir::CImageResizerVars resizeVars;

VkImage::VkImage(const ImageLib::Image &theImage) {
    mWidth = theImage.mWidth;
    mHeight = theImage.mHeight;

    if (!mWidth || !mHeight) throw std::runtime_error("Images with no size are not supported.");

    renderMutex.lock();

    endRenderPass();

    VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

    image = createImage(mWidth, mHeight, flags);
    memory = createImageMemory(image);
    view = createImageView(image, pixelFormat);
    framebuffer = createFramebuffer(view, mWidth, mHeight);
    descriptor = createDescriptorSet(view, textureSampler);

    VkDeviceSize imageSize = mWidth * mHeight * SCALE * SCALE * sizeof(uint32_t);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory
    );

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    if (SCALE != 1) {
        if constexpr (SCALE == 2) {
            resizeVars.IsResize2 = true;
        }

        // Upscaling is done here
        ImageResizer.resizeImage(
            reinterpret_cast<uint8_t *>(theImage.mBits.get()), mWidth, mHeight, 0, static_cast<uint8_t *>(data),
            mWidth * SCALE, mHeight * SCALE, 4, 0, &resizeVars
        );
    } else {
        memcpy(data, theImage.mBits.get(), imageSize);
    }
    vkUnmapMemory(device, stagingBufferMemory);

    uploadNewData(stagingBuffer);

    doDeleteInfo(deleteInfo{{}, {}, {}, stagingBufferMemory, {}, stagingBuffer});

    renderMutex.unlock();
}

// this function must be behind the renderMutex.
void VkImage::uploadNewData(VkBuffer stagingBuffer) {
    endRenderPass();

    TransitionLayout(imageCommandBuffers[imageBufferIdx], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(imageCommandBuffers[imageBufferIdx], stagingBuffer, image, mWidth * SCALE, mHeight * SCALE);
};

VkImage::VkImage(VkImage &theImage) : VkImage(theImage.mWidth, theImage.mHeight, false) {
    applyEffects(&theImage, this, FILTER_EFFECT_NONE);
}

VkImage::VkImage(int width, int height, bool initialise, bool textureRepeat) {
    mWidth = width;
    mHeight = height;

    if (!mWidth || !mHeight) throw std::runtime_error("Images with no size are not supported.");

    renderMutex.lock();

    endRenderPass();

    VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

    image = createImage(mWidth, mHeight, flags);
    memory = createImageMemory(image);
    view = createImageView(image, pixelFormat);
    framebuffer = createFramebuffer(view, mWidth, mHeight);

    if (textureRepeat) {
        descriptor = createDescriptorSet(view, textureSamplerRepeat);
    } else {
        descriptor = createDescriptorSet(view, textureSampler);
    }

    if (initialise) {
        TransitionLayout(imageCommandBuffers[imageBufferIdx], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkClearColorValue color = {
            .uint32{0, 0, 0, 0}
        };

        VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        vkCmdClearColorImage(imageCommandBuffers[imageBufferIdx], image, layout, &color, 1, &range);
    }

    renderMutex.unlock();
}

VkImage::~VkImage() {
    renderMutex.lock();

    doDeleteInfo(deleteInfo{image, view, framebuffer, memory, descriptor, {}});

    renderMutex.unlock();
}

/*====================*
 | GRAPHICS FUNCTIONS |
 *====================*/

constexpr auto accessMaskMap =
    const_generate_sparse_array<std::array<std::pair<VkImageLayout, std::pair<VkAccessFlags, VkPipelineStageFlags>>, 5>{
        {
         {VK_IMAGE_LAYOUT_UNDEFINED, {0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}},
         {VK_IMAGE_LAYOUT_GENERAL,
             {VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT}},
         {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT}},
         {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             {VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT}},
         {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             {VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}},
         }
}>();

void transitionImageLayouts(VkCommandBuffer commandBuffer, std::vector<std::pair<VkImage *, VkImageLayout>> images) {
    std::vector<VkImageMemoryBarrier> barriers;

    VkPipelineStageFlags srcStageMask{};
    VkPipelineStageFlags dstStageMask{};

    for (auto image : images) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = image.first->layout;
        barrier.newLayout = image.second;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image.first->image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        auto srcAccess = accessMaskMap[image.first->layout];
        auto dstAccess = accessMaskMap[image.second];

        barrier.srcAccessMask = srcAccess.first;
        barrier.dstAccessMask = dstAccess.first;
        srcStageMask |= srcAccess.second;
        dstStageMask |= dstAccess.second;

        image.first->layout = image.second;

        barriers.push_back(barrier);
    }

    vkCmdPipelineBarrier(
        commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data()
    );
}

void VkImage::TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
    transitionImageLayouts(
        commandBuffer,
        {
            {this, newLayout}
    }
    );
}

void flushCommandBuffer() {
    endRenderPass();

    vkEndCommandBuffer(imageCommandBuffers[imageBufferIdx]);
    cachedDrawMode = -1;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &imageCommandBuffers[imageBufferIdx];

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, imageFences[imageBufferIdx]);

    imageBufferIdx = (imageBufferIdx + 1) % NUM_IMAGE_SWAPS;
    beginCommandBuffer();
}

void VkImage::applyEffects(VkImage *theSrcImage, VkImage *theDestImage, FilterEffect theFilterEffect) {
    renderMutex.lock();

    endRenderPass();

    constexpr auto newLayout = VK_IMAGE_LAYOUT_GENERAL;

    transitionImageLayouts(
        imageCommandBuffers[imageBufferIdx],
        {
            {theSrcImage,  newLayout},
            {theDestImage, newLayout}
    }
    );

    vkCmdBindPipeline(imageCommandBuffers[imageBufferIdx], VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

    std::array<VkDescriptorSet, 2> descriptorSetsToBind = {theSrcImage->descriptor, theDestImage->descriptor};
    vkCmdBindDescriptorSets(
        imageCommandBuffers[imageBufferIdx], VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 2,
        descriptorSetsToBind.data(), 0, 0
    );

    ComputePushConstants constants = {theFilterEffect};

    vkCmdPushConstants(
        imageCommandBuffers[imageBufferIdx], computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
        sizeof(ComputePushConstants), &constants
    );

    if (theSrcImage->mWidth != theDestImage->mWidth && theSrcImage->mHeight != theDestImage->mHeight) {
        renderMutex.unlock();
        throw std::runtime_error("applyEffectsToImage: The dimensions of the src and dest image don't match.");
    }
    vkCmdDispatch(
        imageCommandBuffers[imageBufferIdx], (SCALE * theSrcImage->mWidth) / 16, (SCALE * theSrcImage->mHeight) / 16, 1
    );

    renderMutex.unlock();
}

std::unique_ptr<VkImage> VkImage::applyEffectsToNewImage(FilterEffect theFilterEffect) {
    auto newImage = std::make_unique<VkImage>(mWidth, mHeight, false);
    newImage->CopyAttributes(this);

    applyEffects(this, newImage.get(), theFilterEffect);

    return newImage;
}

void VkImage::SetViewportAndScissor(const glm::vec4 &theClipRect) {
    VkViewport viewport = {0, 0, static_cast<float>(mWidth * SCALE), static_cast<float>(mHeight * SCALE), 0.0, 1.0};
    vkCmdSetViewport(imageCommandBuffers[imageBufferIdx], 0, 1, &viewport);

    VkRect2D scissor = {
        {static_cast<int32_t>(theClipRect.x * SCALE),  static_cast<int32_t>(theClipRect.y * SCALE) },
        {static_cast<uint32_t>(theClipRect.z * SCALE), static_cast<uint32_t>(theClipRect.w * SCALE)}
    };
    vkCmdSetScissor(imageCommandBuffers[imageBufferIdx], 0, 1, &scissor);
}

void VkImage::BeginDraw(Image *theImage, int theDrawMode) {
    static VkImage *otherCachedImage = nullptr;
    static VkImage *thisCachedImage = nullptr;

    bool otherCacheMiss = false;
    // bool otherLayoutSuboptimal = false;
    VkImage *otherImage = nullptr;

    otherImage = dynamic_cast<VkImage *>(theImage);
    otherCacheMiss = (otherImage != otherCachedImage);
    otherCachedImage = otherImage;
    // otherLayoutSuboptimal = (otherImage->layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    bool thisCacheMiss = (this != thisCachedImage);
    bool drawModeMiss = (theDrawMode != cachedDrawMode);
    cachedDrawMode = theDrawMode;
    thisCachedImage = this;

    // bool thisLayoutSuboptimal =  (layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    if (drawModeMiss) {
        if (theDrawMode == 1) {
            vkCmdBindPipeline(
                imageCommandBuffers[imageBufferIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineAdditive
            );
        } else {
            vkCmdBindPipeline(imageCommandBuffers[imageBufferIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        }
    }

    if (thisCacheMiss || otherCacheMiss) endRenderPass();

    /*
    if (thisLayoutSuboptimal || otherLayoutSuboptimal) {
        std::vector<std::pair<VkImage*, VkImageLayout>> transitions;

        if (thisLayoutSuboptimal) transitions.push_back({this, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        if (otherLayoutSuboptimal) transitions.push_back({otherImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});

        transitionImageLayouts(imageCommandBuffers[imageBufferIdx], transitions);
    }*/

    if (!inRenderpass) {
        // Memory barriers prevent out of order frames, we always need them.
        std::vector<std::pair<VkImage *, VkImageLayout>> transitions;
        transitions.reserve(2);
        transitions.push_back({this, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        transitions.push_back({otherImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});

        transitionImageLayouts(imageCommandBuffers[imageBufferIdx], transitions);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = imagePass;
        renderPassInfo.framebuffer = framebuffer;

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {
            static_cast<uint32_t>(mWidth * SCALE),
            static_cast<uint32_t>(mHeight * SCALE),
        };
        vkCmdBeginRenderPass(imageCommandBuffers[imageBufferIdx], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        inRenderpass = true;

        vkCmdBindDescriptorSets(
            imageCommandBuffers[imageBufferIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
            &otherImage->descriptor, 0, nullptr
        );
    }
}

inline glm::vec4 RectToVec4(Rect a) { return glm::vec4(a.mX, a.mY, a.mWidth, a.mHeight); }

inline constexpr glm::vec4 calcUVs(const glm::vec4 &theSrcRect, const glm::vec2 &extent) {
    return glm::vec4(
        (theSrcRect.x) / extent.x, (theSrcRect.y) / extent.y, (theSrcRect.x + theSrcRect.z) / extent.x,
        (theSrcRect.y + theSrcRect.w) / extent.y
    );
}

constexpr inline std::array<glm::vec4, 4> generateVertices(glm::vec4 dr, glm::vec4 uv, glm::vec2 ex) {
    return {
        {
         {2 * ((dr.x)) / ex.x - 1, 2 * ((dr.y)) / ex.y - 1, uv.x, uv.y},
         {2 * ((dr.x + dr.z)) / ex.x - 1, 2 * ((dr.y)) / ex.y - 1, uv.z, uv.y},
         {2 * ((dr.x)) / ex.x - 1, 2 * ((dr.y + dr.w)) / ex.y - 1, uv.x, uv.w},
         {2 * ((dr.x + dr.z)) / ex.x - 1, 2 * ((dr.y + dr.w)) / ex.y - 1, uv.z, uv.w},
         }
    };
}

/*================*
 | DRAW FUNCTIONS |
 *================*/

std::unique_ptr<VkImage> blankImage;

void VkImage::FillRect(const Rect &theRect, const Color &theColor, int theDrawMode) {
    if (blankImage == nullptr) {
        auto bits = std::make_unique<uint32_t[]>(1);
        bits[0] = 0xFFFFFFFF;
        auto inputImage = ImageLib::Image(1, 1, std::move(bits));
        blankImage = std::make_unique<VkImage>(inputImage);
    }

    Rect theSrcRect = {0, 0, 1, 1};
    StretchBlt(blankImage.get(), theRect, theSrcRect, theRect, theColor, theDrawMode, false);
}

void VkImage::Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode) {
    Rect theClipRect = {theX, theY, theSrcRect.mWidth, theSrcRect.mHeight};

    BltF(theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode);
}

void VkImage::BltF(
    Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor,
    int theDrawMode
) {
    BltEx(
        theImage,
        generateVertices(
            {theX, theY, theSrcRect.mWidth, theSrcRect.mHeight},
            calcUVs(RectToVec4(theSrcRect), glm::vec2(theImage->mWidth, theImage->mHeight)), {mWidth, mHeight}
        ),
        RectToVec4(theClipRect), theColor, theDrawMode, true
    );
}

void VkImage::StretchBlt(
    Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor,
    int theDrawMode, bool
) {
    BltEx(
        theImage,
        generateVertices(
            RectToVec4(theDestRect), calcUVs(RectToVec4(theSrcRect), glm::vec2(theImage->mWidth, theImage->mHeight)),
            {mWidth, mHeight}
        ),
        RectToVec4(theClipRect), theColor, theDrawMode, true
    );
}

void VkImage::BltMatrix(
    Image *theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect &theClipRect, const Color &theColor,
    int theDrawMode, const Rect &theSrcRect, bool blend
) {
    float w2 = theSrcRect.mWidth / 2.0;
    float h2 = theSrcRect.mHeight / 2.0;

    glm::vec4 uv = calcUVs(RectToVec4(theSrcRect), glm::vec2(theImage->mWidth, theImage->mHeight));

    std::array<glm::vec4, 4> vertices = {
        {
         {-w2, -h2, uv.x, uv.y},
         {w2, -h2, uv.z, uv.y},
         {-w2, h2, uv.x, uv.w},
         {w2, h2, uv.z, uv.w},
         }
    };

    for (auto &vert : vertices) {
        SexyVector3 v(vert.x, vert.y, 1);
        v = theMatrix * v;
        vert.x = 2 * ((v.x + x) / mWidth) - 1;
        vert.y = 2 * ((v.y + y) / mHeight) - 1;
    }

    BltEx(theImage, vertices, RectToVec4(theClipRect), theColor, theDrawMode, blend);
}

void VkImage::BltEx(
    Image *theImage, const std::array<glm::vec4, 4> theVertices, const glm::vec4 &theClipRect, const Color &theColor,
    int theDrawMode, bool blend
) {
    if (theClipRect.z <= 0 || theClipRect.w <= 0) return; // Can't draw regions with negative size.
    renderMutex.lock();

    VkImage::BeginDraw(theImage, theDrawMode);

    SetViewportAndScissor(theClipRect);

    SexyRGBA color = theColor.ToRGBA();
    ImagePushConstants constants = {
        {theVertices[0], theVertices[1], theVertices[2], theVertices[3]},
        {color,          color,          color,          color         },
        true, blend
    };

    vkCmdPushConstants(
        imageCommandBuffers[imageBufferIdx], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(ImagePushConstants), &constants
    );

    vkCmdDraw(imageCommandBuffers[imageBufferIdx], 6, 1, 0, 0);

    renderMutex.unlock();
}

void VkImage::BltTrianglesTex(
    Image *theTexture, const std::array<TriVertex, 3> *theVertices, int theNumTriangles, const Rect &theClipRect,
    const Color &theColor, int theDrawMode, float tx, float ty, bool blend
) {
    if (theClipRect.mWidth <= 0 || theClipRect.mHeight <= 0) return; // Can't draw regions with negative size.
    renderMutex.lock();

    VkImage::BeginDraw(theTexture, theDrawMode);

    SetViewportAndScissor(RectToVec4(theClipRect));

    for (int i = 0; i < theNumTriangles; ++i) {
        auto &triangle = theVertices[i];

        auto vertexToNative = [tx, ty, &mWidth = mWidth, &mHeight = mHeight](const TriVertex &v) {
            return glm::vec4(2 * (v.x + tx) / mWidth - 1, 2 * (v.y + ty) / mHeight - 1, v.u, v.v);
        };

        auto colorFromInt = [theColor](uint32_t c) { return c ? Color(c).ToRGBA() : theColor.ToRGBA(); };

        ImagePushConstants constants = {
            {vertexToNative(triangle[0]),     vertexToNative(triangle[1]),     vertexToNative(triangle[2]),     glm::vec4(0)},
            {colorFromInt(triangle[0].color), colorFromInt(triangle[1].color), colorFromInt(triangle[2].color), {}          },
            false,
            blend
        };

        vkCmdPushConstants(
            imageCommandBuffers[imageBufferIdx], pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ImagePushConstants), &constants
        );
        vkCmdDraw(imageCommandBuffers[imageBufferIdx], 3, 1, 0, 0);
    }

    renderMutex.unlock();
}

/*=================*
 |  UNIMPLEMENTED  |
 *=================*/

void VkImage::BltRotated(Image *, float, float, const Rect &, const Rect &, const Color &, int, double, float, float) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::BltRotated is a stub.\n");
    has_shown = true;
}

void VkImage::BltMirror(Image *, int, int, const Rect &, const Color &, int) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::BltMirror is a stub.\n");
    has_shown = true;
}

void VkImage::StretchBltMirror(Image *, const Rect &, const Rect &, const Rect &, const Color &, int, bool) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::StretchBltMirror is a stub.\n");
    has_shown = true;
}

void VkImage::ClearRect(const Rect & /*theRect*/) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::ClearRect is a stub.\n");
    has_shown = true;
}

void VkImage::DrawLine(double, double, double, double, const Color &, int) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::DrawLine is a stub.\n");
    has_shown = true;
}

void VkImage::DrawLineAA(double, double, double, double, const Color &, int) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::DrawLineAA is a stub.\n");
    has_shown = true;
}

void VkImage::FillScanLinesWithCoverage(Span *, int, const Color &, int, const unsigned char *, int, int, int, int) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::FillScanLinesWithCoverage is a stub.\n");
    has_shown = true;
}

bool VkImage::PolyFill3D(const Point *, int, const Rect *, const Color &, int, int, int) {
    static bool has_shown = false;
    if (!has_shown) printf("draw:     VkImage::PolyFill3D is a stub.\n");
    has_shown = true;

    return false;
}
} // namespace Vk

/*
void Image::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const
BYTE* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight); bool Image::PolyFill3D(const
Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int
ty, bool convex); void Image::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor,
int theDrawMode); void Image::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect
&theClipRect, const Color& theColor, int theDrawMode); void Image::BltRotated(Image* theImage, float theX, float theY,
const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float
theRotCenterX, float theRotCenterY); void Image::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect&
theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch); void
Image::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color&
theColor, int theDrawMode, const Rect &theSrcRect, bool blend); void Image::BltTrianglesTex(Image *theTexture, const
TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float
tx, float ty, bool blend); void Image::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const
Color& theColor, int theDrawMode); void Image::StretchBltMirror(Image* theImage, const Rect& theDestRect, const Rect&
theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);
*/
