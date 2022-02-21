#pragma once

#include "GraphicsImage.h"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rkrai {
class GraphicsImageView {
    public:
    GraphicsImageView(GraphicsImage& graphicsImage, vk::ImageAspectFlags aspectFlags);
    GraphicsImageView(const GraphicsImageView&) = delete;
    void operator=(const GraphicsImageView&) = delete;
    GraphicsImageView(GraphicsImageView&&) = default;
    GraphicsImageView& operator=(GraphicsImageView&&) = delete;

    vk::ImageView getImageView() { return *imageView; }

    private:
    GraphicsImage& graphicsImage;

    vk::UniqueImageView imageView;
    vk::ImageAspectFlags aspectFlags;

    void createImageView();
};
}