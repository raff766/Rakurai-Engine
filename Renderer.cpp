#include "Renderer.h"

#include <stdexcept>

Renderer::Renderer(Window& window, GraphicsDevice& device) : window(window), graphicsDevice(device) {
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer() {
    freeCommandBuffers();
}

void Renderer::recreateSwapChain() {
    while (window.getWidth() == 0 || window.getHeight() == 0) {
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(graphicsDevice.getDevice());

    VkExtent2D extent = {static_cast<uint32_t>(window.getWidth()), static_cast<uint32_t>(window.getHeight())};
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

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphicsDevice.getCommandPool();
    allocInfo.commandBufferCount = commandBuffers.size();

    if (vkAllocateCommandBuffers(graphicsDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void Renderer::freeCommandBuffers() {
    vkFreeCommandBuffers(graphicsDevice.getDevice(), graphicsDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
    commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
    assert(!isFrameStarted && "Cant call begin frame while already in progress!");
    VkResult result;

    result = swapChain->acquireNextImage(currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    isFrameStarted = true;

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin renderpass on a command buffer from a different frame!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    VkClearValue clearValues[2];
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    VkExtent2D swapChainExtent = swapChain->getExtent();
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{{0, 0}, swapChainExtent};

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end renderpass on a command buffer from a different frame!");
    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::endFrame() {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress!");
    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }

    VkResult result = swapChain->submitDrawCommands(&commandBuffer);
    if ( result != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    result = swapChain->presentImage(currentImageIndex);
    if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasFramebufferResized()) {
        window.resetFramebufferResizedFlag();
        recreateSwapChain();
    } else if ( result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}