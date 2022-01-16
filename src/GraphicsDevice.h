#pragma once

#include "Window.h"

#define VULKAN_HPP_NO_NODISCARD_WARNINGS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <vector>
#include <optional>
#include <utility>

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class GraphicsDevice {
    public:
    GraphicsDevice(Window& window);
    GraphicsDevice(const GraphicsDevice&) = delete;
    void operator=(const GraphicsDevice&) = delete;

    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    vk::Device getDevice() { return *device; }
    vk::SurfaceKHR getSurface() { return *surface; }
    QueueFamilyIndices getQueueFamilyIndices() { return findQueueFamilyIndices(physicalDevice); }
    SwapChainSupportDetails getSwapChainSupportDetails() { return getSwapChainSupportDetails(physicalDevice); }
    vk::CommandPool getCommandPool() { return *commandPool; }
    vk::Queue getGraphicsQueue() { return graphicsQueue; }
    vk::Queue getPresentQueue() { return presentQueue; }

    private:
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const bool validationLayersEnabled = true;
    
    Window& window;
    vk::UniqueInstance instance;
    vk::UniqueDevice device;
    vk::UniqueDebugUtilsMessengerEXT debugMessenger;
    vk::UniqueSurfaceKHR surface;
    vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::UniqueCommandPool commandPool;

    void createInstance();
    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();
    void destroyDebugMessenger();
    vk::DebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo();

    void createSurface();

    void pickPhysicalDevice();
    int rateDeviceSuitability(vk::PhysicalDevice device);
    bool hasRequiredDeviceExtensions(vk::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilyIndices(vk::PhysicalDevice device);

    void createLogicalDevice();

    SwapChainSupportDetails getSwapChainSupportDetails(vk::PhysicalDevice device);

    void createCommandPool();

    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(vk::CommandBuffer commandBuffer);
};