#pragma once

#include "Camera.h"
#include "GraphicsDevice.h"
#include "GraphicsPipeline.h"
#include "GameObject.h"

#include <memory>
#include <vector>

class SimpleRenderSystem {
public:
    SimpleRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass);
    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    void operator=(const SimpleRenderSystem&) = delete;

    void renderGameObjects(vk::CommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

private:
    void createPipelineLayout();
    void createPipeline(vk::RenderPass renderPass);

    GraphicsDevice& graphicsDevice;
    
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    vk::UniquePipelineLayout pipelineLayout;
};