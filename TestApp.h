#pragma once

#include "Window.h"
#include "GraphicsDevice.h"
#include "GraphicsPipeline.h"
#include "SwapChain.h"
#include "Model.h"

#include <memory>
#include <vector>

class TestApp {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    TestApp();
    ~TestApp();

    TestApp(const TestApp&) = delete;
    void operator=(const TestApp&) = delete;

    void run();
    
private:
    void loadModels();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    Window window{WIDTH, HEIGHT, "Test App"};
    GraphicsDevice graphicsDevice{window};
    SwapChain swapChain{graphicsDevice, {WIDTH, HEIGHT}};
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;
    std::unique_ptr<Model> model;
};