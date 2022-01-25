#include "Descriptors.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vector>

namespace rkrai {
Descriptors::Descriptors(
        GraphicsDevice& device,
        vk::DescriptorType descriptorType,
        vk::ShaderStageFlagBits shaderStages,
        uint32_t setCount,
        uint32_t descriptorsPerSet,
        uint32_t binding) 
        : graphicsDevice(device), descriptorType(descriptorType), shaderStages(shaderStages), setCount(setCount), 
        descriptorsPerSet(descriptorsPerSet), binding(binding) {
    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSets();
}

void Descriptors::populateDescriptorSet(size_t index, GraphicsBuffer& buffer) {
    vk::DescriptorBufferInfo bufferInfo{buffer.getBuffer(), 0, VK_WHOLE_SIZE};
    graphicsDevice.getDevice().updateDescriptorSets(
        vk::WriteDescriptorSet{descriptorSets[index], 0, 0, descriptorType, {}, bufferInfo}, {}
    );
}

void Descriptors::createDescriptorPool() {
    vk::DescriptorPoolSize poolSize{descriptorType, descriptorsPerSet * setCount};
    descriptorPool = graphicsDevice.getDevice().createDescriptorPoolUnique({{}, setCount, poolSize});
}

void Descriptors::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding descriptorSetBinding{binding, descriptorType, descriptorsPerSet, shaderStages, {}};
    descriptorSetLayout = graphicsDevice.getDevice().createDescriptorSetLayoutUnique({{}, descriptorSetBinding});
}

void Descriptors::allocateDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(setCount, *descriptorSetLayout);
    descriptorSets = graphicsDevice.getDevice().allocateDescriptorSets({*descriptorPool, layouts});
}
}