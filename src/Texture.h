#pragma once

#include "GraphicsDevice.h"
#include "Image.h"
#include "ImageView.h"

#include <optional>
#include <string>
#include <tuple>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rkrai {
class Texture {
    public:
    Texture(GraphicsDevice& graphicsDevice, std::string imageFilePath);

    private:
    GraphicsDevice& graphicsDevice;

    std::optional<Image> image;
    std::optional<ImageView> imageView;

    vk::UniqueSampler sampler;

    std::tuple<std::byte*, vk::DeviceSize, vk::Extent3D> loadTextureFile(std::string path);
    void createSampler();
};
}