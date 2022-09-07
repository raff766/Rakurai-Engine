#pragma once

#include "GraphicsBuffer.h"
#include "Texture.h"

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rkrai {
class ResourceBinder {
    public:
    struct Binding {
        uint32_t index;
        vk::DescriptorType descriptorType;
        uint32_t descriptorCount;
    };

    ResourceBinder(GraphicsDevice& graphicsDevice, std::vector<Binding> bindings);
    ResourceBinder(const ResourceBinder&) = delete;
    void operator=(const ResourceBinder&) = delete;
    ResourceBinder(ResourceBinder&&) = default;
    ResourceBinder& operator=(ResourceBinder&&) = delete;

    void setBuffer(uint32_t index, GraphicsBuffer* graphicsBuffer);
    void setTexture(uint32_t index, Texture* texture);
    void bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t setNum);

    vk::DescriptorSetLayout getSetLayout() { return *descriptorSetLayout; }

    private:
    GraphicsDevice& graphicsDevice;

    std::vector<Binding> bindings;

    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;

    void createDescriptorPool();
    void createDescriptorSetLayout();
    void allocateDescriptorSet();
};
}