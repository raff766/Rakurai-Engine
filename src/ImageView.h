#pragma once

#include "Image.h"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rkrai {
class ImageView {
    public:
    ImageView(Image& image, vk::ImageAspectFlags aspectFlags);
    ImageView(const ImageView&) = delete;
    void operator=(const ImageView&) = delete;
    ImageView(ImageView&&) = default;
    ImageView& operator=(ImageView&&) = delete;

    vk::ImageView getImageView() { return *imageView; }

    private:
    Image& image;

    vk::UniqueImageView imageView;
    vk::ImageAspectFlags aspectFlags;

    void createImageView();
};
}