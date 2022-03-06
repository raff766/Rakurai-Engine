#include "ResourceBinder.h"
#include "GraphicsBuffer.h"

#include <cstdint>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rkrai {
void ResourceBinder::add(GraphicsBuffer* graphicsBuffer, uint32_t binding, vk::DescriptorType descriptorType) {
    graphicsBuffers.insert(std::make_pair(descriptorType, std::make_pair(graphicsBuffer, binding)));
}

void ResourceBinder::finalize() {
    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSet();
    populateDescriptorSet();
}

void ResourceBinder::bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout) {
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        0,
        descriptorSet,
        {}
    );
}

void ResourceBinder::createDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes;
    //Loop through every unique key, aka DescriptorType, within graphicsBuffers
    for (auto it = graphicsBuffers.begin(); it != graphicsBuffers.end(); it = graphicsBuffers.equal_range(it->first).second) {
        vk::DescriptorType descriptorType = it->first;
        poolSizes.emplace_back(descriptorType, static_cast<uint32_t>(graphicsBuffers.count(descriptorType)));
    }
    descriptorPool = graphicsDevice.getDevice().createDescriptorPoolUnique({{}, 1, poolSizes});
}

void ResourceBinder::createDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> setBindings;
    for (auto it = graphicsBuffers.begin(); it != graphicsBuffers.end(); it = graphicsBuffers.equal_range(it->first).second) {
        vk::DescriptorType descriptorType = it->first;
        uint32_t descriptorCount = static_cast<uint32_t>(graphicsBuffers.count(descriptorType));
        setBindings.emplace_back(it->second.second, descriptorType, descriptorCount, vk::ShaderStageFlagBits::eAll);
    }
    descriptorSetLayout = graphicsDevice.getDevice().createDescriptorSetLayoutUnique({{}, setBindings});
}

void ResourceBinder::allocateDescriptorSet() {
    descriptorSet = graphicsDevice.getDevice().allocateDescriptorSets({*descriptorPool, *descriptorSetLayout})[0];
}

void ResourceBinder::populateDescriptorSet() {
    for (const auto&[descriptorType, bufferAndBinding] : graphicsBuffers) {
        vk::DescriptorBufferInfo bufferInfo{bufferAndBinding.first->getBuffer(), 0, VK_WHOLE_SIZE};
        graphicsDevice.getDevice().updateDescriptorSets(
            vk::WriteDescriptorSet{descriptorSet, bufferAndBinding.second, 0, descriptorType, {}, bufferInfo}, {}
        );
    }
}
}