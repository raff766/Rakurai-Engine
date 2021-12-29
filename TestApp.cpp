#include "TestApp.h"

#include <stdexcept>

TestApp::TestApp() {
    loadModels();
    createPipelineLayout();
    createPipeline();
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
    VkExtent2D swapChainExtent = swapChain.getExtent();

    PipelineConfigInfo pipelineConfig = GraphicsPipeline::getDefaultPipelineConfigInfo(swapChainExtent.width, swapChainExtent.height);
    pipelineConfig.renderPass = swapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        graphicsDevice,
        "shaders/SimpleShader.vert.spv",
        "shaders/SimpleShader.frag.spv",
        pipelineConfig
    );
}

void TestApp::createCommandBuffers() {
    commandBuffers.resize(swapChain.getImageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphicsDevice.getCommandPool();
    allocInfo.commandBufferCount = commandBuffers.size();

    if (vkAllocateCommandBuffers(graphicsDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    for (int i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain.getRenderPass();
        renderPassInfo.framebuffer = swapChain.getFrameBuffer(i);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.getExtent();

        VkClearValue clearValue;
        clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        graphicsPipeline->bind(commandBuffers[i]);
        model->bind(commandBuffers[i]);
        model->draw(commandBuffers[i]);

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
}

void TestApp::drawFrame() {
    uint32_t imageIndex = swapChain.acquireNextImage();
    swapChain.submitDrawCommands(&commandBuffers[imageIndex]);
    swapChain.presentImage(imageIndex);
}