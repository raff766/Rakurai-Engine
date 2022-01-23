#pragma once

#include "Camera.h"
#include "GraphicsDevice.h"
#include "GraphicsPipeline.h"
#include "GameObject.h"
#include "Descriptors.h"

#include <memory>
#include <optional>
#include <vector>

namespace rkrai {
class SimpleRenderSystem {
public:
    SimpleRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass, std::vector<GraphicsBuffer>& globalUboBuffers);
    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    void operator=(const SimpleRenderSystem&) = delete;

    void bindGlobalUbo(vk::CommandBuffer commandBuffer, int frameIndex);
    void renderGameObjects(vk::CommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

private:
    void createGlobalUboDescriptors();
    void createPipelineLayout();
    void createPipeline(vk::RenderPass renderPass);

    GraphicsDevice& graphicsDevice;
    std::vector<GraphicsBuffer>& globalUboBuffers;
    
    std::optional<Descriptors> descriptors;
    vk::UniquePipelineLayout pipelineLayout;
    std::optional<GraphicsPipeline> graphicsPipeline;
};
}