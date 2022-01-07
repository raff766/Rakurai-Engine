#pragma once

#include "Camera.h"
#include "GraphicsDevice.h"
#include "GraphicsPipeline.h"
#include "GameObject.h"

#include <memory>
#include <vector>

class SimpleRenderSystem {
public:
    SimpleRenderSystem(GraphicsDevice& device, VkRenderPass renderPass);
    ~SimpleRenderSystem();
    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    void operator=(const SimpleRenderSystem&) = delete;

    void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    GraphicsDevice& graphicsDevice;
    
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    VkPipelineLayout pipelineLayout;
};