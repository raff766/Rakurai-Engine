#pragma once

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

    Window window{WIDTH, HEIGHT, "Test App"};
    GraphicsDevice graphicsDevice{window};
    Renderer renderer{window, graphicsDevice};
    
    std::vector<GameObject> gameObjects;
};