#include "ImageView.h"

#include <vulkan/vulkan_structs.hpp>

namespace rkrai {
ImageView::ImageView(Image& image, vk::ImageAspectFlags aspectFlags)
: image(image), aspectFlags(aspectFlags) {
    createImageView();
}

void ImageView::createImageView() {
    imageView = image.graphicsDevice.getDevice().createImageViewUnique({
        {}, image.getImage(), vk::ImageViewType::e2D, image.imageFormat, {},
        vk::ImageSubresourceRange{aspectFlags, 0, 1, 0, 1}
    });
}
}