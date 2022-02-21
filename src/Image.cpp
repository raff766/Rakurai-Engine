#include "Image.h"
#include "GraphicsBuffer.h"
#include "GraphicsCommands.h"
#include "GraphicsDevice.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rkrai {
Image::Image(
    GraphicsDevice& device, vk::ImageType imageType, vk::Extent3D imageExtent, vk::Format imageFormat, vk::ImageUsageFlags imageUsage) 
    : graphicsDevice(device), imageType(imageType), imageExtent(imageExtent), imageFormat(imageFormat), imageUsage(imageUsage) {
    createImage();
    allocateImageMemory();
}

Image::Image(
    GraphicsDevice& device, std::string imageFilePath)
    : graphicsDevice(device), imageType(vk::ImageType::e2D), imageFormat(vk::Format::eR8G8B8A8Srgb),
    imageUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled) {
    loadTextureFile(imageFilePath);
}

void Image::loadTextureFile(std::string path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * (sizeof(stbi_uc) * 4);

    if (!pixels) {
        throw std::runtime_error("Failed to load texture file!");
    }

    imageExtent = vk::Extent3D{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};
    createImage();
    allocateImageMemory();

    GraphicsBuffer stagingBuffer{
        graphicsDevice,
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    stagingBuffer.mapData(pixels);

    stbi_image_free(pixels);

    transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    stagingBuffer.copyToImage(*image, imageExtent.width, imageExtent.height);
    transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Image::createImage() {
    vk::ImageCreateInfo imageInfo{
        {}, imageType, imageFormat, imageExtent, 1, 1,
        vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, imageUsage, vk::SharingMode::eExclusive
    };

    image = graphicsDevice.getDevice().createImageUnique(imageInfo);
}

void Image::allocateImageMemory() {
    vk::MemoryRequirements memRequiremnts = graphicsDevice.getDevice().getImageMemoryRequirements(*image);
    uint32_t memType = graphicsDevice.findMemoryType(memRequiremnts.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    imageMemory = graphicsDevice.getDevice().allocateMemoryUnique({memRequiremnts.size, memType});

    graphicsDevice.getDevice().bindImageMemory(*image, *imageMemory, 0);
}

void Image::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    vk::ImageMemoryBarrier barrier{
        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eNone,
        oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, *image,
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    vk::PipelineStageFlagBits sourceStage;
    vk::PipelineStageFlagBits destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    GraphicsCommands::submitSingleTimeCommand(graphicsDevice, [&](vk::CommandBuffer commandBuffer){
        commandBuffer.pipelineBarrier(
            sourceStage, destinationStage,
            vk::DependencyFlagBits::eByRegion,
            {}, {}, barrier
        );
    });
}
}