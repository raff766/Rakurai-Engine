#include "Image.h"
#include "GraphicsBuffer.h"
#include "GraphicsCommands.h"
#include "GraphicsDevice.h"

#include <vulkan/vulkan.hpp>
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

void Image::loadData(std::byte* data, vk::DeviceSize size) {
    GraphicsBuffer stagingBuffer{
        graphicsDevice,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    stagingBuffer.mapData(data);

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