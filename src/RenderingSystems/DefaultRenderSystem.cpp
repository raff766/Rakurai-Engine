#include "DefaultRenderSystem.h"
#include "GraphicsPipeline.h"
#include "Model.h"
#include "ResourceBinder.h"
#include "SwapChain.h"
#include <vector>
#include <vulkan/vulkan_handles.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <optional>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace rkrai {

#define MAX_POINT_LIGHTS 10

struct SimplePushConstantData {
    glm::mat4 modelMat{1.0f};
    glm::mat4 normalMat{1.0f};
};

struct PointLight {
    glm::vec4 position{0.0f}; //w is ignored
    glm::vec4 color{1.0f}; //w is intensity
};

struct SimpleUbo {
    glm::mat4 projMat{1.0f};
    glm::mat4 viewMat{1.0f};
    glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; //w is intensity
    PointLight pointLights[MAX_POINT_LIGHTS];
    int numLights;
};

DefaultRenderSystem::DefaultRenderSystem(GraphicsDevice& device, vk::RenderPass renderPass, std::shared_ptr<const Camera> camera)
    : graphicsDevice(device), renderPass(renderPass), camera(camera) {
    createUboBuffers();
    createResourceBinder();
    createPipelineLayout();
    createPipeline();
}

void DefaultRenderSystem::createUboBuffers() {
    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        uboBuffers.emplace_back(
            graphicsDevice,
            sizeof(SimpleUbo),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible
        );
    }
}

void DefaultRenderSystem::createResourceBinder() {
    for (int i = 0; i < uboBuffers.size(); i++) {
        resourceBinder.emplace_back(
            graphicsDevice,
            std::vector<ResourceBinder::Binding>{ {0, vk::DescriptorType::eUniformBuffer, 1} }
        );
        resourceBinder[i].setBuffer(0, &uboBuffers[i]);
    }
    perObjectBinder.emplace(
        graphicsDevice,
        std::vector<ResourceBinder::Binding>{ {1, vk::DescriptorType::eCombinedImageSampler, 1} }
    );
}

void DefaultRenderSystem::createPipelineLayout() {
    vk::PushConstantRange pushConstantRange{
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(SimplePushConstantData)
    };
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.push_back(resourceBinder[0].getSetLayout());
    descriptorSetLayouts.push_back(perObjectBinder->getSetLayout());
    pipelineLayout = graphicsDevice.getDevice().createPipelineLayoutUnique({{}, descriptorSetLayouts, pushConstantRange});
}

void DefaultRenderSystem::createPipeline() {
    PipelineConfigInfo pipelineConfig = GraphicsPipeline::getDefaultPipelineConfigInfo();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = *pipelineLayout;
    pipelineConfig.bindingDescriptions = Model::Vertex::getBindingDescriptions();
    pipelineConfig.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
    graphicsPipeline.emplace(
        graphicsDevice,
        "shaders/SimpleShader.vert.spv",
        "shaders/SimpleShader.frag.spv",
        pipelineConfig
    );
}

void DefaultRenderSystem::render(vk::CommandBuffer commandBuffer, int currentFrameIndex) {
    SimpleUbo simpleUbo{
        .projMat = camera->getProjection(),
        .viewMat = camera->getView()
    };

    for (const auto& gameObj : gameObjects) {
        if (gameObj->pointLight != nullptr) {
            PointLight pointLight{
                .position = glm::vec4{gameObj->transform.translation, 1.0f},
                .color = gameObj->pointLight->color
            };
            simpleUbo.pointLights[simpleUbo.numLights] = pointLight;
            simpleUbo.numLights++;
        }
    }

    uboBuffers[currentFrameIndex].mapData(&simpleUbo);

    graphicsPipeline->bind(commandBuffer);
    resourceBinder[currentFrameIndex].bind(commandBuffer, *pipelineLayout, 0);

    for (const auto& gameObj : gameObjects) {
        if (gameObj->model == nullptr) continue;
        SimplePushConstantData push{};
        push.modelMat = gameObj->transform.modelMatrix();
        push.normalMat = gameObj->transform.normalMatrix();
        
        commandBuffer.pushConstants(
            *pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(SimplePushConstantData), &push
        );

        perObjectBinder->setTexture(1, gameObj->texture.get());
        perObjectBinder->bind(commandBuffer, *pipelineLayout, 1);

        gameObj->model->bind(commandBuffer);
        gameObj->model->draw(commandBuffer);
    }
    simpleUbo.numLights = 0;
}
}