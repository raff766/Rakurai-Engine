#include "Window.h"

#include <stdexcept>

Window::Window(int width, int height, std::string name) 
    : width(width), height(height), windowName(name) {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfwWindow = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}