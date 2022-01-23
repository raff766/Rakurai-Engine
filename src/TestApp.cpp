#include "TestApp.h"
#include "SimpleRenderSystem.h"
#include "Camera.h"
#include "MovementController.h"
#include "GraphicsBuffer.h"
#include "SwapChain.h"

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

struct GlobalUbo {
    glm::mat4 projMat{1.0f};
    glm::mat4 viewMat{1.0f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
};

TestApp::TestApp() {
    loadGameObjects();
}

void TestApp::run() {
    rkrai::Camera camera{};
    rkrai::GameObject cameraObject = rkrai::GameObject::createGameObject();
    rkrai::MovementController cameraController{};

    std::vector<rkrai::GraphicsBuffer> globalUboBuffers;
    for (int i = 0; i < rkrai::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        globalUboBuffers.emplace_back(
            graphicsDevice,
            sizeof(GlobalUbo),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible
        );
    }
    rkrai::SimpleRenderSystem simpleRenderSystem{graphicsDevice, renderer.getSwapChainRenderPass(), globalUboBuffers};

    auto currentTime = std::chrono::high_resolution_clock::now();
    while(!window.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window.getGLFWWindow(), frameTime, cameraObject);
        camera.setViewYXZ(cameraObject.transform.translation, cameraObject.transform.rotation);
        camera.setPerspectiveProjection(50.0f, renderer.getAspectRatio(), 0.1f, 1000.0f);
        
        if (auto commandBuffer = renderer.beginFrame()) {
            GlobalUbo globalUbo{};
            globalUbo.projMat = camera.getProjection();
            globalUbo.viewMat = camera.getView();
            globalUboBuffers[renderer.getCurrentFrameIndex()].mapData(&globalUbo);

            renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.bindGlobalUbo(commandBuffer, renderer.getCurrentFrameIndex());
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(graphicsDevice.getDevice());
}

void TestApp::loadGameObjects() {
    std::shared_ptr<rkrai::Model> model = std::make_shared<rkrai::Model>(graphicsDevice, "models/smooth_vase.obj");

    rkrai::GameObject gameObj = rkrai::GameObject::createGameObject();
    gameObj.model = model;
    gameObj.transform.translation = {0.0f, 0.0f, 2.5f};
    gameObj.transform.scale = {1.0f, 1.0f, 1.0f};
    gameObjects.push_back(std::move(gameObj));
}