#pragma once

#include "GraphicsDevice.h"
#include "GraphicsPipeline.h"
#include "SwapChain.h"
#include "GameObject.h"

#include <memory>
#include <vector>

class SimpleRenderSystem {
public:
    SimpleRenderSystem(GraphicsDevice& device, VkRenderPass renderPass);
    ~SimpleRenderSystem();
    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    void operator=(const SimpleRenderSystem&) = delete;

    void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects);

private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    GraphicsDevice& graphicsDevice;
    
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    VkPipelineLayout pipelineLayout;
};