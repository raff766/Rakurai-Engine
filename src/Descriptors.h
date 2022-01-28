#pragma once

#include "GraphicsDevice.h"
#include "GraphicsBuffer.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rkrai {
class Descriptors {
    public:
    Descriptors(
        GraphicsDevice& device,
        vk::DescriptorType descriptorType,
        vk::ShaderStageFlags shaderStages,
        uint32_t setCount,
        uint32_t descriptorsPerSet,
        uint32_t binding);
    Descriptors(const Descriptors&) = delete;
    void operator=(const Descriptors&) = delete;

    void populateDescriptorSet(size_t index, GraphicsBuffer& buffer);

    vk::DescriptorSetLayout getSetLayout() { return *descriptorSetLayout; }
    std::vector<vk::DescriptorSet> getDescriptorSets() { return descriptorSets; }

    private:
    GraphicsDevice& graphicsDevice;
    vk::DescriptorType descriptorType;
    vk::ShaderStageFlags shaderStages;
    uint32_t setCount;
    uint32_t descriptorsPerSet;
    uint32_t binding;
    
    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    std::vector<vk::DescriptorSet> descriptorSets;

    void createDescriptorPool();
    void createDescriptorSetLayout();
    void allocateDescriptorSets();
};
}