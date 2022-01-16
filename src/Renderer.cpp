#include "Renderer.h"

#include <stdexcept>

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

vk::CommandBuffer Renderer::beginFrame() {
    assert(!isFrameStarted && "Cant call begin frame while already in progress!");
    
    vk::ResultValue<uint32_t> result = swapChain->acquireNextImage();
    currentImageIndex = result.value;
    if (result.result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return nullptr;
    } else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    isFrameStarted = true;

    vk::CommandBuffer commandBuffer = getCurrentCommandBuffer();
    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    return commandBuffer;
}

void Renderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin renderpass on a command buffer from a different frame!");

    vk::Extent2D swapChainExtent = swapChain->getExtent();

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
    renderPassInfo.renderArea = {{0, 0}, swapChainExtent};
    
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    renderPassInfo.setClearValues(clearValues);

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport{
        0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f
    };
    vk::Rect2D scissor{{0, 0}, swapChainExtent};

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);
}

void Renderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end renderpass on a command buffer from a different frame!");
    commandBuffer.endRenderPass();
}

void Renderer::endFrame() {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress!");
    vk::CommandBuffer commandBuffer = getCurrentCommandBuffer();
    commandBuffer.end();

    swapChain->submitDrawCommands(commandBuffer);

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