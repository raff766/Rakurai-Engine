#pragma once

#include "RenderSystem.h"
#include "Window.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <cassert>
#include <memory>
#include <optional>
#include <vector>

namespace rkrai {
class Renderer {
public:
    Renderer(Window& window, GraphicsDevice& device);
    Renderer(const Renderer&) = delete;
    void operator=(const Renderer&) = delete;

    void setRenderSystem(std::shared_ptr<RenderSystem> renderSystem) { this->renderSystem = renderSystem; }
    void drawFrame();

    vk::RenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
    float getAspectRatio() const { return swapChain->getAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted; }

private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    bool beginFrame();
    void endFrame();
    void beginSwapChainRenderPass();
    void endSwapChainRenderPass();

    Window& window;
    GraphicsDevice& graphicsDevice;

    std::unique_ptr<SwapChain> swapChain;
    std::vector<vk::UniqueCommandBuffer> commandBuffers;
    uint32_t currentImageIndex = 0;
    int currentFrameIndex = 0;
    bool isFrameStarted = false;

    std::shared_ptr<RenderSystem> renderSystem;
};
}