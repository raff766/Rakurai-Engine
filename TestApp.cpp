#include "TestApp.h"
#include "SimpleRenderSystem.h"
#include "Camera.h"
#include "MovementController.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>
#include <stdexcept>

TestApp::TestApp() {
    loadGameObjects();
}

TestApp::~TestApp() {}

void TestApp::run() {
    SimpleRenderSystem simpleRenderSystem{graphicsDevice, renderer.getSwapChainRenderPass()};
    Camera camera{};
    GameObject cameraObject = GameObject::createGameObject();
    MovementController cameraController{};
    auto currentTime = std::chrono::high_resolution_clock::now();
    while(!window.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window.getGLFWWindow(), frameTime, cameraObject);
        camera.setViewYXZ(cameraObject.transform.translation, cameraObject.transform.rotation);
        camera.setPerspectiveProjection(50.0f, renderer.getAspectRatio(), 0.1f, 10.0f);
        
        if (auto commandBuffer = renderer.beginFrame()) {
            renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(graphicsDevice.getDevice());
}

void TestApp::loadGameObjects() {
    std::shared_ptr<Model> model = Model::createModelFromFile(graphicsDevice, "models/smooth_vase.obj");

    GameObject gameObj = GameObject::createGameObject();
    gameObj.model = model;
    gameObj.transform.translation = {0.0f, 0.0f, 2.5f};
    gameObj.transform.scale = {1.0f, 1.0f, 1.0f};
    gameObjects.push_back(std::move(gameObj));
}