#include "SimpleRenderSystem.h"
#include "Descriptors.h"
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
struct SimplePushConstantData {
    glm::mat4 modelMat{1.0f};
    glm::mat4 normalMat{1.0f};
};

struct GlobalUbo {
    glm::mat4 projMat{1.0f};
    glm::mat4 viewMat{1.0f};
    glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; //w is intensity
    glm::vec4 lightColor{1.0f};
    glm::vec3 lightPosition{0.0f, -1.0f, 1.0f};
    //glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
};

SimpleRenderSystem::SimpleRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass, std::shared_ptr<const Camera> camera)
    : graphicsDevice(device), renderPass(renderPass), camera(camera) {
    createGlobalUboBuffers();
    createGlobalUboDescriptors();
    createPipelineLayout();
    createPipeline();
}

void SimpleRenderSystem::createGlobalUboBuffers() {
    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        globalUboBuffers.emplace_back(
            graphicsDevice,
            sizeof(GlobalUbo),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible
        );
    }
}

void SimpleRenderSystem::createGlobalUboDescriptors() {
    descriptors.emplace(
        graphicsDevice,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
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
    vk::DescriptorSetLayout descriptorSetLayout = descriptors->getSetLayout();
    pipelineLayout = graphicsDevice.getDevice().createPipelineLayoutUnique({{}, descriptorSetLayout, pushConstantRange});
}

void SimpleRenderSystem::createPipeline() {
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

void SimpleRenderSystem::render(vk::CommandBuffer commandBuffer, int currentFrameIndex) {
    GlobalUbo globalUbo{};
    globalUbo.projMat = camera->getProjection();
    globalUbo.viewMat = camera->getView();
    globalUboBuffers[currentFrameIndex].mapData(&globalUbo);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, descriptors->getDescriptorSets()[currentFrameIndex], {});
    graphicsPipeline->bind(commandBuffer);

    for (auto& gameObj : gameObjects) {
        SimplePushConstantData push{};
        push.modelMat = gameObj->transform.modelMatrix();
        push.normalMat = gameObj->transform.normalMatrix();
        
        commandBuffer.pushConstants(
            *pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(SimplePushConstantData), &push
        );
        gameObj->model->bind(commandBuffer);
        gameObj->model->draw(commandBuffer);
    }
}
}