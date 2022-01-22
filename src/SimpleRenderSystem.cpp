#include "SimpleRenderSystem.h"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <optional>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

struct SimplePushConstantData {
    glm::mat4 modelMat{1.0f};
    glm::mat4 normalMat{1.0f};
};

SimpleRenderSystem::SimpleRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass, std::vector<GraphicsBuffer>& globalUboBuffers)
    : graphicsDevice(device), globalUboBuffers(globalUboBuffers) {
    createGlobalUboDescriptors();
    createPipelineLayout();
    createPipeline(renderPass);
}

void SimpleRenderSystem::createGlobalUboDescriptors() {
    descriptors.emplace(
        graphicsDevice,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex,
        globalUboBuffers.size(),
        1,
        0
    );
    for (int i = 0; i < globalUboBuffers.size(); i++) {
        descriptors->populateDescriptorSet(i, globalUboBuffers[i]);
    }
}

void SimpleRenderSystem::createPipelineLayout() {
    vk::PushConstantRange pushConstantRange{
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(SimplePushConstantData)
    };
    vk::DescriptorSetLayout setLayout = descriptors->getSetLayout();
    pipelineLayout = graphicsDevice.getDevice().createPipelineLayoutUnique({{}, setLayout, pushConstantRange});
}

void SimpleRenderSystem::createPipeline(vk::RenderPass renderPass) {
    PipelineConfigInfo pipelineConfig = GraphicsPipeline::getDefaultPipelineConfigInfo();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = *pipelineLayout;
    graphicsPipeline.emplace(
        graphicsDevice,
        "shaders/SimpleShader.vert.spv",
        "shaders/SimpleShader.frag.spv",
        pipelineConfig
    );
}

void SimpleRenderSystem::bindGlobalUbo(vk::CommandBuffer commandBuffer, int frameIndex) {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, descriptors->getDescriptorSets()[frameIndex], {});
}

void SimpleRenderSystem::renderGameObjects(vk::CommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera) {
    graphicsPipeline->bind(commandBuffer);

    glm::mat4 projectionView = camera.getProjection() * camera.getView();

    for (auto& obj : gameObjects) {
        SimplePushConstantData push{};
        push.modelMat = obj.transform.modelMatrix();
        push.normalMat = obj.transform.normalMatrix();
        
        commandBuffer.pushConstants(
            *pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(SimplePushConstantData), &push
        );
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}