#pragma once

#include "Window.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"

#include <cassert>
#include <memory>
#include <vector>

class Renderer {
public:
    Renderer(Window& window, GraphicsDevice& device);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    void operator=(const Renderer&) = delete;

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
    float getAspectRatio() const { return swapChain->getAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted; }
    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get command buffer when frame is not in progress!");
        return commandBuffers[currentFrameIndex];
    }
    int getFrameIndex() const {
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
    std::vector<VkCommandBuffer> commandBuffers;
    uint32_t currentImageIndex = 0;
    int currentFrameIndex = 0;
    bool isFrameStarted = false;
};