#include "TestApp.h"

#include <stdexcept>

TestApp::TestApp() {
    loadModels();
    createPipelineLayout();
    recreateSwapChain();
    createCommandBuffers();
}

TestApp::~TestApp() {
    vkDestroyPipelineLayout(graphicsDevice.getDevice(), pipelineLayout, nullptr);
}

void TestApp::run() {
    while(!window.shouldClose()) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(graphicsDevice.getDevice());
}

void TestApp::loadModels() {
    std::vector<Model::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    model = std::make_unique<Model>(graphicsDevice, vertices);
}

void TestApp::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(graphicsDevice.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void TestApp::createPipeline() {
    VkExtent2D swapChainExtent = swapChain->getExtent();

    PipelineConfigInfo pipelineConfig = GraphicsPipeline::getDefaultPipelineConfigInfo();
    pipelineConfig.renderPass = swapChain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        graphicsDevice,
        "shaders/SimpleShader.vert.spv",
        "shaders/SimpleShader.frag.spv",
        pipelineConfig
    );
}

void TestApp::recreateSwapChain() {
    while (window.getWidth() == 0 || window.getHeight() == 0) {
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(graphicsDevice.getDevice());

    VkExtent2D extent = {static_cast<uint32_t>(window.getWidth()), static_cast<uint32_t>(window.getHeight())};
    if (swapChain == nullptr) {
        swapChain = std::make_unique<SwapChain>(graphicsDevice, extent);
    } else {
        swapChain = std::make_unique<SwapChain>(graphicsDevice, extent, std::move(swapChain));
        if (swapChain->getImageCount() != commandBuffers.size()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }
    //TODO: check if render pass if compatible in order to not recreate pipleine everytime with swapchain
    createPipeline();
}

void TestApp::createCommandBuffers() {
    commandBuffers.resize(swapChain->getImageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphicsDevice.getCommandPool();
    allocInfo.commandBufferCount = commandBuffers.size();

    if (vkAllocateCommandBuffers(graphicsDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void TestApp::freeCommandBuffers() {
    vkFreeCommandBuffers(graphicsDevice.getDevice(), graphicsDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
    commandBuffers.clear();
}

void TestApp::recordCommandBuffer(int imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(imageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    VkClearValue clearValue;
    clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    VkExtent2D swapChainExtent = swapChain->getExtent();
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{{0, 0}, swapChainExtent};

    vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

    graphicsPipeline->bind(commandBuffers[imageIndex]);
    model->bind(commandBuffers[imageIndex]);
    model->draw(commandBuffers[imageIndex]);

    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void TestApp::drawFrame() {
    uint32_t imageIndex = 0;
    VkResult result;

    result = swapChain->acquireNextImage(imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    recordCommandBuffer(imageIndex);

    result = swapChain->submitDrawCommands(&commandBuffers[imageIndex]);
    if ( result != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    result = swapChain->presentImage(imageIndex);
    if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasFramebufferResized()) {
        window.resetFramebufferResizedFlag();
        recreateSwapChain();
        return;
    } else if ( result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }
}