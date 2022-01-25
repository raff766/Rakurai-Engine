#include "GraphicsCommands.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace rkrai {
void GraphicsCommands::submitSingleTimeCommand(GraphicsDevice& device, std::function<void(vk::CommandBuffer)> command) {
    vk::UniqueCommandBuffer commandBuffer = std::move(device.getDevice().allocateCommandBuffersUnique(
        {device.getCommandPool(), vk::CommandBufferLevel::ePrimary, 1}
    )[0]);

    commandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    command(*commandBuffer);
    commandBuffer->end();

    vk::SubmitInfo submitInfo{{}, {}, *commandBuffer};
    device.getGraphicsQueue().submit(submitInfo);
    device.getGraphicsQueue().waitIdle();
}
}