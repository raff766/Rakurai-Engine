#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace rkrai {
class RenderSystem {
    private:
    virtual void render(vk::CommandBuffer commandBuffer, int currentFrameIndex) = 0;

    friend class Renderer;
};
}