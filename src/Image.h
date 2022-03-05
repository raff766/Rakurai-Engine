#pragma once

#include "GraphicsDevice.h"

#include <cstddef>
#include <string>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rkrai {
class Image {
    public:
    Image(
        GraphicsDevice& device,
        vk::ImageType imageType,
        vk::Extent3D imageExtent,
        vk::Format imageFormat,
        vk::ImageUsageFlags imageUsage
    );
    Image(const Image&) = delete;
    void operator=(const Image&) = delete;
    Image(Image&&) = default;
    Image& operator=(Image&&) = delete;

    void loadData(std::byte* data, vk::DeviceSize size);

    vk::Image getImage() { return *image; }

    private:
    GraphicsDevice& graphicsDevice;

    vk::UniqueImage image;
    vk::UniqueDeviceMemory imageMemory;

    vk::ImageType imageType;
    vk::Extent3D imageExtent;
    vk::Format imageFormat;
    vk::ImageUsageFlags imageUsage;

    void createImage();
    void allocateImageMemory();
    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

    friend class ImageView;
};
}