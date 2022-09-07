#include "ResourceBinder.h"
#include "GraphicsBuffer.h"
#include "GraphicsDevice.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rkrai {
ResourceBinder::ResourceBinder(GraphicsDevice& graphicsDevice, std::vector<Binding> bindings) 
: graphicsDevice(graphicsDevice), bindings(std::move(bindings)) {
    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSet();
}

void ResourceBinder::setBuffer(uint32_t index, GraphicsBuffer* graphicsBuffer) {
    std::optional<vk::DescriptorType> descriptorType;
    for (const auto& binding : bindings) {
        if (binding.index == index) {
            descriptorType = binding.descriptorType;
        }
    }
    if (!descriptorType) throw std::runtime_error("This binding index does not exist!");

    vk::DescriptorBufferInfo bufferInfo{graphicsBuffer->getBuffer(), 0, VK_WHOLE_SIZE};
    graphicsDevice.getDevice().updateDescriptorSets(
        vk::WriteDescriptorSet{descriptorSet, index, 0, *descriptorType, {}, bufferInfo}, {}
    );
}

void ResourceBinder::setTexture(uint32_t index, Texture* texture) {
    std::optional<vk::DescriptorType> descriptorType;
    for (const auto& binding : bindings) {
        if (binding.index == index) {
            descriptorType = binding.descriptorType;
        }
    }
    if (!descriptorType) throw std::runtime_error("This binding index does not exist!");

    vk::DescriptorImageInfo imageInfo{texture->getSampler(), texture->getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal};
    graphicsDevice.getDevice().updateDescriptorSets(
        vk::WriteDescriptorSet{descriptorSet, index, 0, *descriptorType, imageInfo}, {}
    );
}

void ResourceBinder::bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t setNum) {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, setNum, descriptorSet, {});
}

void ResourceBinder::createDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes;
    for (const Binding& binding : bindings) {
        poolSizes.emplace_back(binding.descriptorType, binding.descriptorCount);
    }
    descriptorPool = graphicsDevice.getDevice().createDescriptorPoolUnique({{}, 1, poolSizes});
}

void ResourceBinder::createDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> setBindings;
    for (const Binding& binding : bindings) {
        setBindings.emplace_back(binding.index, binding.descriptorType, binding.descriptorCount, vk::ShaderStageFlagBits::eAll);
    }
    descriptorSetLayout = graphicsDevice.getDevice().createDescriptorSetLayoutUnique({{}, setBindings});
}

void ResourceBinder::allocateDescriptorSet() {
    descriptorSet = graphicsDevice.getDevice().allocateDescriptorSets({*descriptorPool, *descriptorSetLayout})[0];
}
}