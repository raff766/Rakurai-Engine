#include "BillboardRenderSystem.h"
#include "GraphicsPipeline.h"
#include "ResourceBinder.h"
#include "SwapChain.h"
#include <vector>
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
        resourceBinder.emplace_back(
            graphicsDevice,
            std::vector<ResourceBinder::Binding>{ {0, vk::DescriptorType::eUniformBuffer, 1} }
        );
        resourceBinder[i].setBuffer(0, &uboBuffers[i]);
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
    BillboardUbo billboardUbo{
        .projMat = camera->getProjection(),
        .viewMat = camera->getView()
    };
    uboBuffers[currentFrameIndex].mapData(&billboardUbo);

    graphicsPipeline->bind(commandBuffer);
    resourceBinder[currentFrameIndex].bind(commandBuffer, *pipelineLayout, 0);

    for (const auto& gameObject : gameObjects) {
        BillboardPushConstantData push{
            .billboardColor = gameObject->billboard->color,
            .billboardDimensions = gameObject->billboard->dimensions,
            .billboardPosition = gameObject->transform.translation
        };

        commandBuffer.pushConstants(
            *pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(BillboardPushConstantData), &push
        );

        commandBuffer.draw(6, 1, 0, 0);
    }
}
}