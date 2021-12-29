#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, std::string name);
    ~Window();

    Window(const Window&) = delete;
    void operator=(const Window&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(glfwWindow); };
    
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
    void initWindow();

    const int width;
    const int height;

    std::string windowName;
    GLFWwindow* glfwWindow;
};
