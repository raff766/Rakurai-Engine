#pragma once

#include "Camera.h"
#include "GraphicsDevice.h"
#include "GraphicsPipeline.h"
#include "GameObject.h"
#include "RenderSystem.h"
#include "ResourceBinder.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace rkrai {
class DefaultRenderSystem : public RenderSystem {
public:
    DefaultRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass, std::shared_ptr<const Camera> camera);
    DefaultRenderSystem(const DefaultRenderSystem&) = delete;
    void operator=(const DefaultRenderSystem&) = delete;

    void addGameObject(std::shared_ptr<const GameObject> gameObject) { gameObjects.push_back(gameObject); }
    void removeGameObject();
    void setCamera(std::shared_ptr<const Camera> camera) { this->camera = camera; }
    GraphicsPipeline& getPipeline() { return *graphicsPipeline; }

private:
    void createUboBuffers();
    void createResourceBinder();
    void createPipelineLayout();
    void createPipeline();
    void render(vk::CommandBuffer commandBuffer, int currentFrameIndex);

    GraphicsDevice& graphicsDevice;
    vk::RenderPass renderPass;

    std::vector<std::shared_ptr<const GameObject>> gameObjects;
    std::shared_ptr<const Camera> camera;
    
    std::vector<GraphicsBuffer> uboBuffers;
    std::vector<ResourceBinder> resourceBinder;
    std::optional<ResourceBinder> perObjectBinder;
    vk::UniquePipelineLayout pipelineLayout;
    std::optional<GraphicsPipeline> graphicsPipeline;
};
}