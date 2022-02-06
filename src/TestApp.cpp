#include "TestApp.h"
#include "Camera.h"
#include "GameObject.h"
#include "Model.h"
#include "MovementController.h"
#include "GraphicsBuffer.h"
#include "RenderingSystems/BillboardRenderSystem.h"
#include "RenderingSystems/SimpleRenderSystem.h"
#include "Renderer.h"
#include "SwapChain.h"

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <chrono>
#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

TestApp::TestApp() {
    loadGameObjects();
}

void TestApp::run() {
    auto camera = std::make_shared<rkrai::Camera>();
    auto simpleRenderSystem = std::make_shared<rkrai::SimpleRenderSystem>(graphicsDevice, renderer.getSwapChainRenderPass(), camera);
    auto billboardRenderSystem = std::make_shared<rkrai::BillboardRenderSystem>(graphicsDevice, renderer.getSwapChainRenderPass(), camera);
    rkrai::GameObject cameraObject{};
    rkrai::MovementController cameraController{};
    
    for (const auto& gameObj : gameObjects) {
        if (gameObj->model != nullptr) {
            simpleRenderSystem->addGameObject(gameObj);
        } else if (gameObj->billboard != nullptr) {
            billboardRenderSystem->addGameObject(gameObj);
        }
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    renderer.addRenderSystem(simpleRenderSystem);
    renderer.addRenderSystem(billboardRenderSystem);
    while(!window.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window.getGLFWWindow(), frameTime, cameraObject);
        camera->setPerspectiveProjection(50.0f, renderer.getAspectRatio(), 0.1f, 1000.0f);
        camera->setViewYXZ(cameraObject.transform.translation, cameraObject.transform.rotation);

        renderer.drawFrame();
    }

    vkDeviceWaitIdle(graphicsDevice.getDevice());
}

void TestApp::loadGameObjects() {
    auto model = std::make_shared<rkrai::Model>(graphicsDevice, "models/smooth_vase.obj");
    auto gameObj = std::make_shared<rkrai::GameObject>();
    gameObj->model = model;
    gameObj->transform.translation = {0.0f, 0.0f, 2.5f};
    gameObj->transform.scale = {1.0f, 1.0f, 1.0f};
    gameObjects.push_back(gameObj);

    auto floorModel = std::make_shared<rkrai::Model>(graphicsDevice, "models/quad.obj");
    auto floor = std::make_shared<rkrai::GameObject>();
    floor->model = floorModel;
    floor->transform.translation = {0.0f, 0.0f, 2.5f};
    floor->transform.scale = {3.0f, 1.0f, 3.0f};
    gameObjects.push_back(floor);

    auto lightBillboard = std::make_shared<rkrai::Billboard>();
    auto light = std::make_shared<rkrai::GameObject>();
    lightBillboard->billboardColor = glm::vec4{1.0f};
    lightBillboard->billboardDimensions = glm::vec2{0.1f};
    light->billboard = lightBillboard;
    light->transform.translation = {0.0f, -1.0f, 1.0f};
    gameObjects.push_back(light);
}