#pragma once

#include "GraphicsDevice.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <functional>

namespace rkrai {
class GraphicsCommands {
    public:
    static void submitSingleTimeCommand(GraphicsDevice& device, std::function<void(vk::CommandBuffer)> command);

    private:
};
}