#pragma once

#include "Window.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <utility>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
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
    ~GraphicsDevice();

    GraphicsDevice(const GraphicsDevice&) = delete;
    void operator=(const GraphicsDevice&) = delete;

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& memory);
    void createImage(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkDevice getDevice() { return device; }
    VkSurfaceKHR getSurface() { return surface; }
    QueueFamilyIndices getQueueFamilyIndices() { return findQueueFamilyIndices(physicalDevice); }
    SwapChainSupportDetails getSwapChainSupportDetails() { return getSwapChainSupportDetails(physicalDevice); }
    VkCommandPool getCommandPool() { return commandPool; }
    VkQueue getGraphicsQueue() { return graphicsQueue; }
    VkQueue getPresentQueue() { return presentQueue; }

    private:
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const bool validationLayersEnabled = true;
    
    Window& window;
    VkDevice device;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;

    void createInstance();
    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();
    void setupDebugMessenger();
    void destroyDebugMessenger();
    VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo();

    void createSurface();

    void pickPhysicalDevice();
    int rateDeviceSuitability(VkPhysicalDevice device);
    bool hasRequiredDeviceExtensions(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device);

    void createLogicalDevice();

    SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice device);

    void createCommandPool();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};