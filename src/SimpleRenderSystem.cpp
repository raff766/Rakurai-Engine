#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

struct SimplePushConstantData {
    glm::mat4 transform{1.0f};
    glm::mat4 normalMatrix{1.0f};
};

SimpleRenderSystem::SimpleRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass) : graphicsDevice(device) {
    createPipelineLayout();
    createPipeline(renderPass);
}

void SimpleRenderSystem::createPipelineLayout() {
    vk::PushConstantRange pushConstantRange{
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(SimplePushConstantData)
    };

    pipelineLayout = graphicsDevice.getDevice().createPipelineLayoutUnique({{}, {}, pushConstantRange});
}

void SimpleRenderSystem::createPipeline(vk::RenderPass renderPass) {
    PipelineConfigInfo pipelineConfig = GraphicsPipeline::getDefaultPipelineConfigInfo();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = *pipelineLayout;
    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        graphicsDevice,
        "shaders/SimpleShader.vert.spv",
        "shaders/SimpleShader.frag.spv",
        pipelineConfig
    );
}

void SimpleRenderSystem::renderGameObjects(vk::CommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera) {
    graphicsPipeline->bind(commandBuffer);

    glm::mat4 projectionView = camera.getProjection() * camera.getView();

    for (auto& obj : gameObjects) {
        SimplePushConstantData push{};
        push.transform = projectionView * obj.transform.modelMatrix();
        push.normalMatrix = obj.transform.normalMatrix();
        
        commandBuffer.pushConstants(
            *pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(SimplePushConstantData), &push
        );
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}