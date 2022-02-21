#include "GraphicsBuffer.h"
#include "GraphicsDevice.h"
#include "GraphicsCommands.h"
#include <vulkan/vulkan_enums.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rkrai {
GraphicsBuffer::GraphicsBuffer(GraphicsDevice& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    : graphicsDevice(device), size(size) {
    createBuffer(size, usage, properties);
}

void GraphicsBuffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    vk::Device device = graphicsDevice.getDevice();

    buffer = device.createBufferUnique({{}, size, usage, vk::SharingMode::eExclusive});

    vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(*buffer);
    bufferMemory = device.allocateMemoryUnique({
        memRequirements.size, 
        graphicsDevice.findMemoryType(memRequirements.memoryTypeBits, properties)
    });
    device.bindBufferMemory(*buffer, *bufferMemory, 0);
}

void GraphicsBuffer::copyFrom(const GraphicsBuffer& srcBuffer) {
    GraphicsCommands::submitSingleTimeCommand(graphicsDevice, [&](vk::CommandBuffer commandBuffer){
        commandBuffer.copyBuffer(*srcBuffer.buffer, *buffer, vk::BufferCopy{0, 0, srcBuffer.size});
    });
}

void GraphicsBuffer::mapData(const void* data) {
    void* pointer = graphicsDevice.getDevice().mapMemory(*bufferMemory, 0, size);
    memcpy(pointer, data, size);
    graphicsDevice.getDevice().unmapMemory(*bufferMemory);
}

void GraphicsBuffer::copyToImage(vk::Image image, uint32_t width, uint32_t height) {
    vk::BufferImageCopy region{
        0, 0, 0,
        vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        vk::Offset3D{0, 0, 0},
        vk::Extent3D{width, height, 1}
    };
    
    GraphicsCommands::submitSingleTimeCommand(graphicsDevice, [&](vk::CommandBuffer commandBuffer){
        commandBuffer.copyBufferToImage(*buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    });
}
}