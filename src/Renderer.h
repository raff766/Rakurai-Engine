#pragma once

#include "Window.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"

#include <cassert>
#include <memory>
#include <vector>

namespace rkrai {
class Renderer {
public:
    Renderer(Window& window, GraphicsDevice& device);
    Renderer(const Renderer&) = delete;
    void operator=(const Renderer&) = delete;

    vk::CommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
    void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);

    vk::RenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
    float getAspectRatio() const { return swapChain->getAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted; }
    vk::CommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get command buffer when frame is not in progress!");
        return *commandBuffers[currentFrameIndex];
    }
    int getCurrentFrameIndex() const {
        assert(isFrameStarted && "Cannot get frame index when frame not in progress!");
        return currentFrameIndex;
    }

private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    Window& window;
    GraphicsDevice& graphicsDevice;
    std::unique_ptr<SwapChain> swapChain;
    std::vector<vk::UniqueCommandBuffer> commandBuffers;
    uint32_t currentImageIndex = 0;
    int currentFrameIndex = 0;
    bool isFrameStarted = false;
};
}