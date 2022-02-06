#include "Renderer.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <stdexcept>
#include <cassert>

namespace rkrai {
Renderer::Renderer(Window& window, GraphicsDevice& device) : window(window), graphicsDevice(device) {
    recreateSwapChain();
    createCommandBuffers();
}

void Renderer::recreateSwapChain() {
    while (window.getWidth() == 0 || window.getHeight() == 0) {
        glfwWaitEvents();
    }
    graphicsDevice.getDevice().waitIdle();

    vk::Extent2D extent = {static_cast<uint32_t>(window.getWidth()), static_cast<uint32_t>(window.getHeight())};
    if (swapChain == nullptr) {
        swapChain = std::make_unique<SwapChain>(graphicsDevice, extent);
    } else {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
        swapChain = std::make_unique<SwapChain>(graphicsDevice, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapChainFormats(*swapChain)) {
            throw std::runtime_error("Swap chain image or depth format has changed!");
        }
    }
    //TODO: check if render pass if compatible in order to not recreate pipleine everytime with swapchain
}

void Renderer::createCommandBuffers() {
    commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocInfo{
        graphicsDevice.getCommandPool(), vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(commandBuffers.size())
    };

    commandBuffers = graphicsDevice.getDevice().allocateCommandBuffersUnique(allocInfo);
}

void Renderer::drawFrame() {
    //assert(renderSystems != nullptr && "A RenderSystem must be set before attempting to draw frames.");

    if (beginFrame()) {
        beginSwapChainRenderPass();
        for (auto& renderSystem : renderSystems) {
            renderSystem->render(*commandBuffers[currentFrameIndex], currentFrameIndex);
        }
        endSwapChainRenderPass();
        endFrame();
    }
}

bool Renderer::beginFrame() {
    vk::ResultValue<uint32_t> result = swapChain->acquireNextImage();
    currentImageIndex = result.value;
    if (result.result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return false;
    } else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    isFrameStarted = true;
    commandBuffers[currentFrameIndex]->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    return true;
}

void Renderer::beginSwapChainRenderPass() {
    vk::Extent2D swapChainExtent = swapChain->getExtent();

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
    renderPassInfo.renderArea = {{0, 0}, swapChainExtent};
    
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    renderPassInfo.setClearValues(clearValues);

    commandBuffers[currentFrameIndex]->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport{
        0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f
    };
    vk::Rect2D scissor{{0, 0}, swapChainExtent};

    commandBuffers[currentFrameIndex]->setViewport(0, viewport);
    commandBuffers[currentFrameIndex]->setScissor(0, scissor);
}

void Renderer::endSwapChainRenderPass() {
    commandBuffers[currentFrameIndex]->endRenderPass();
}

void Renderer::endFrame() {
    commandBuffers[currentFrameIndex]->end();

    swapChain->submitDrawCommands(*commandBuffers[currentFrameIndex]);

    vk::Result result = swapChain->presentImage(currentImageIndex);
    if ( result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.wasFramebufferResized()) {
        window.resetFramebufferResizedFlag();
        recreateSwapChain();
    } else if ( result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}
}