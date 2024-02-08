#ifndef VK_COMMON_H
#define VK_COMMON_H

#include "Color.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#define SCALE 2

namespace Vk {

extern VkDevice device;

class VkImage;

constexpr int NUM_IMAGE_SWAPS = 3;

constexpr VkFormat pixelFormat = VK_FORMAT_B8G8R8A8_UNORM;

extern std::mutex renderMutex;

extern VkQueue graphicsQueue;
extern VkPipeline graphicsPipeline;
extern VkPipeline computePipeline;
extern VkPipeline graphicsPipelineAdditive;
extern std::array<VkCommandBuffer, NUM_IMAGE_SWAPS> imageCommandBuffers;
extern VkRenderPass imagePass;
extern std::array<VkFence, NUM_IMAGE_SWAPS> imageFences;

extern VkDescriptorPool descriptorPool;
extern VkDescriptorSetLayout descriptorSetLayout;

// extern std::vector<VkBuffer> uniformBuffers;
// extern VkBuffer vertexBuffer;
// extern VkBuffer indexBuffer;

extern VkPipelineLayout pipelineLayout;
extern VkPipelineLayout computePipelineLayout;
extern VkSampler textureSampler;
extern VkSampler textureSamplerRepeat;

extern uint32_t currentFrame;
// extern VkImage* otherCachedImage;

extern const std::vector<uint16_t> indices;

extern VkPhysicalDeviceProperties physicalDeviceProperties;

VkImageView createImageView(::VkImage image, VkFormat format);

struct ImagePushConstants {
    glm::vec4 vertices[4];
    Sexy::SexyRGBA vertex_colors[4];
    uint32_t is_quad;
    uint32_t filter;
    glm::vec2 filterOffset = glm::vec2();
};

struct ComputePushConstants {
    int32_t type;
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

template <decltype(auto) arr> static constexpr auto const_generate_sparse_array() {
    using T = decltype(arr)::value_type::second_type;

    constexpr auto max_key = std::max_element(arr.begin(), arr.end(), [](const auto &left, const auto &right) {
                                 return left.first < right.first;
                             })->first;

    std::array<T, max_key + 1> sparse_array{};

    for (auto it : arr) {
        sparse_array[it.first] = it.second;
    }

    return sparse_array;
}

void createBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
    VkDeviceMemory &bufferMemory
);

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void flushCommandBuffer();
void copyBufferToImage(
    VkCommandBuffer commandBuffer, VkBuffer buffer, ::VkImage image, uint32_t width, uint32_t height
);
void beginCommandBuffer();

struct deleteInfo {
    std::optional<::VkImage> image;
    std::optional<VkImageView> view;
    std::optional<VkFramebuffer> framebuffer;
    std::optional<VkDeviceMemory> memory;
    std::optional<VkDescriptorSet> set;
    std::optional<VkBuffer> buffer;
};
void doDeleteInfo(deleteInfo info);
void deferredDelete(size_t idx);

} // namespace Vk

#endif // VK_COMMON_H