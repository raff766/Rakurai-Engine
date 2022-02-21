#include "GraphicsImageView.h"

#include <vulkan/vulkan_structs.hpp>

namespace rkrai {
GraphicsImageView::GraphicsImageView(GraphicsImage& graphicsImage, vk::ImageAspectFlags aspectFlags)
: graphicsImage(graphicsImage), aspectFlags(aspectFlags) {
    createImageView();
}

void GraphicsImageView::createImageView() {
    imageView = graphicsImage.graphicsDevice.getDevice().createImageViewUnique({
        {}, *graphicsImage.image, vk::ImageViewType::e2D, graphicsImage.imageFormat, {},
        vk::ImageSubresourceRange{aspectFlags, 0, 1, 0, 1}
    });
}
}