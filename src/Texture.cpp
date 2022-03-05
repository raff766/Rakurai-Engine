#include "Texture.h"
#include "Image.h"
#include "ImageView.h"

#include <cstddef>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>

namespace rkrai {
Texture::Texture(GraphicsDevice& graphicsDevice, std::string imageFilePath) : graphicsDevice(graphicsDevice) {
    auto[data, size, extent] = loadTextureFile(imageFilePath);
    image.emplace(graphicsDevice, vk::ImageType::e2D, extent, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst);
    imageView.emplace(*image, vk::ImageAspectFlagBits::eColor);
    image->loadData(data, size);
    stbi_image_free(data);
}

std::tuple<std::byte*, vk::DeviceSize, vk::Extent3D> Texture::loadTextureFile(std::string path) {
    int texWidth, texHeight, texChannels;
    std::byte* pixels = reinterpret_cast<std::byte*>(stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha));
    vk::DeviceSize imageSize = texWidth * texHeight * (sizeof(stbi_uc) * 4);

    if (!pixels) {
        throw std::runtime_error("Failed to load texture file!");
    }

    vk::Extent3D imageExtent{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};
    return std::make_tuple(pixels, imageSize, imageExtent);
}

void Texture::createSampler() {
    graphicsDevice.getDevice().createSamplerUnique(vk::SamplerCreateInfo{
        {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
        0.0f, VK_TRUE, graphicsDevice.getDeviceProperties().limits.maxSamplerAnisotropy,
        VK_FALSE, vk::CompareOp::eAlways, 0.0f, 0.0f, vk::BorderColor::eIntOpaqueBlack, VK_FALSE
    });
}
}