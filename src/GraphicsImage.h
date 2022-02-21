#pragma once

#include "GraphicsDevice.h"

#include <string>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rkrai {
class GraphicsImage {
    public:
    GraphicsImage(
        GraphicsDevice& device,
        vk::ImageType imageType,
        vk::Extent3D imageExtent,
        vk::Format imageFormat,
        vk::ImageUsageFlags imageUsage
    );
    GraphicsImage(
        GraphicsDevice& device,
        std::string imageFilePath
    );
    GraphicsImage(const GraphicsImage&) = delete;
    void operator=(const GraphicsImage&) = delete;
    GraphicsImage(GraphicsImage&&) = default;
    GraphicsImage& operator=(GraphicsImage&&) = delete;

    void loadTextureFile(std::string path);

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

    friend class GraphicsImageView;
};
}