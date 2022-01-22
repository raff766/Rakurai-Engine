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

    bool shouldClose() const { return glfwWindowShouldClose(glfwWindow); }
    bool wasFramebufferResized() const { return framebufferResized; }
    void resetFramebufferResizedFlag() { framebufferResized = false; }
    GLFWwindow* getGLFWWindow() const { return glfwWindow; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
    static void windowResizedCallback(GLFWwindow* glfwWindow, int width, int height);
    void initWindow();

    int width;
    int height;
    bool framebufferResized = false;

    std::string windowName;
    GLFWwindow* glfwWindow;
};
