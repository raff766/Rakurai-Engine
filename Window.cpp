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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    glfwWindow = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, windowResizedCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void Window::windowResizedCallback(GLFWwindow* glfwWindow, int width, int height) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->framebufferResized = true;
    window->width = width;
    window->height = height;
}