#pragma once

#include "GraphicsDevice.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace rkrai {
class GraphicsBuffer {
    public:
    GraphicsBuffer(GraphicsDevice& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    GraphicsBuffer(const GraphicsBuffer&) = delete;
    void operator=(const GraphicsBuffer&) = delete;
    GraphicsBuffer(GraphicsBuffer&&) = default;
    GraphicsBuffer& operator=(GraphicsBuffer&&) = delete;

    void copyFrom(const GraphicsBuffer& srcBuffer);
    void mapData(const void* data);

    vk::DeviceSize getSize() { return size; }
    vk::Buffer getBuffer() { return *buffer; }
    vk::DeviceMemory getMemory() { return *bufferMemory; }

    private:
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    GraphicsDevice& graphicsDevice;
    vk::DeviceSize size;

    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory bufferMemory;
};
}