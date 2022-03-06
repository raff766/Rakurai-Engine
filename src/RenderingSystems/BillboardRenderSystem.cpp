#include "BillboardRenderSystem.h"
#include "GraphicsPipeline.h"
#include "SwapChain.h"
#include <vulkan/vulkan_enums.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <optional>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace rkrai {
struct BillboardPushConstantData {
    glm::vec4 billboardColor{1.0f}; //w is intensity
    glm::vec2 billboardDimensions{0.1f};
    alignas(16) glm::vec3 billboardPosition{0.0f};
};

struct BillboardUbo {
    glm::mat4 projMat{1.0f};
    glm::mat4 viewMat{1.0f};
};

BillboardRenderSystem::BillboardRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass, std::shared_ptr<const Camera> camera)
    : graphicsDevice(device), renderPass(renderPass), camera(camera) {
    createUboBuffers();
    createResourceBinder();
    createPipelineLayout();
    createPipeline();
}

void BillboardRenderSystem::createUboBuffers() {
    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        uboBuffers.emplace_back(
            graphicsDevice,
            sizeof(BillboardUbo),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible
        );
    }
}

void BillboardRenderSystem::createResourceBinder() {
    for (int i = 0; i < uboBuffers.size(); i++) {
        resourceBinder.emplace_back(graphicsDevice);
        resourceBinder[i].add(&uboBuffers[i], 0, vk::DescriptorType::eUniformBuffer);
        resourceBinder[i].finalize();
    }
}

void BillboardRenderSystem::createPipelineLayout() {
    vk::PushConstantRange pushConstantRange{
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(BillboardPushConstantData)
    };
    vk::DescriptorSetLayout descriptorSetLayout = resourceBinder[0].getSetLayout();
    pipelineLayout = graphicsDevice.getDevice().createPipelineLayoutUnique({{}, descriptorSetLayout, pushConstantRange});
}

void BillboardRenderSystem::createPipeline() {
    PipelineConfigInfo pipelineConfig = GraphicsPipeline::getDefaultPipelineConfigInfo();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = *pipelineLayout;
    graphicsPipeline.emplace(
        graphicsDevice,
        "shaders/BillboardShader.vert.spv",
        "shaders/BillboardShader.frag.spv",
        pipelineConfig
    );
}

void BillboardRenderSystem::render(vk::CommandBuffer commandBuffer, int currentFrameIndex) {
    BillboardUbo billboardUbo{};
    billboardUbo.projMat = camera->getProjection();
    billboardUbo.viewMat = camera->getView();
    uboBuffers[currentFrameIndex].mapData(&billboardUbo);

    graphicsPipeline->bind(commandBuffer);
    resourceBinder[currentFrameIndex].bind(commandBuffer, *pipelineLayout);

    for (const auto& gameObject : gameObjects) {
        BillboardPushConstantData push{};
        push.billboardColor = gameObject->billboard->color;
        push.billboardDimensions = gameObject->billboard->dimensions;
        push.billboardPosition = gameObject->transform.translation;

        commandBuffer.pushConstants(
            *pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(BillboardPushConstantData), &push
        );

        commandBuffer.draw(6, 1, 0, 0);
    }
}
}