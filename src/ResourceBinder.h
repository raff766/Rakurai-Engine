#pragma once

#include "GraphicsBuffer.h"

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
    ResourceBinder(GraphicsDevice& graphicsDevice) : graphicsDevice(graphicsDevice) {}
    ResourceBinder(const ResourceBinder&) = delete;
    void operator=(const ResourceBinder&) = delete;
    ResourceBinder(ResourceBinder&&) = default;
    ResourceBinder& operator=(ResourceBinder&&) = delete;

    void add(GraphicsBuffer* graphicsBuffer, uint32_t binding, vk::DescriptorType descriptorType);
    void finalize();
    void bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);

    vk::DescriptorSetLayout getSetLayout() { return *descriptorSetLayout; }

    private:
    GraphicsDevice& graphicsDevice;

    std::unordered_multimap<vk::DescriptorType, std::pair<GraphicsBuffer*, uint32_t>> graphicsBuffers;

    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;

    void createDescriptorPool();
    void createDescriptorSetLayout();
    void allocateDescriptorSet();
    void populateDescriptorSet();
};
}