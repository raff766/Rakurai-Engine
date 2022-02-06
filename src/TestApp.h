#pragma once

#include "RenderingSystems/SimpleRenderSystem.h"
#include "Window.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"
#include "GameObject.h"
#include "Renderer.h"

#include <memory>
#include <vector>

class TestApp {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    TestApp();

    TestApp(const TestApp&) = delete;
    void operator=(const TestApp&) = delete;

    void run();
    
private:
    void loadGameObjects();

    rkrai::Window window{WIDTH, HEIGHT, "Test App"};
    rkrai::GraphicsDevice graphicsDevice{window};
    rkrai::Renderer renderer{window, graphicsDevice};
    
    std::vector<std::shared_ptr<rkrai::GameObject>> gameObjects;
};