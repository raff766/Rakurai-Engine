#include "GraphicsDevice.h"
#include "SwapChain.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string.h>
#include <map>
#include <set>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

GraphicsDevice::GraphicsDevice(Window& window) : window(window) {
    vk::DynamicLoader dl;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    createInstance();
    if (validationLayersEnabled) {
        debugMessenger = instance->createDebugUtilsMessengerEXTUnique(getDebugMessengerCreateInfo());
    }
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

void GraphicsDevice::createInstance() {
    if (validationLayersEnabled && !checkValidationLayerSupport()) {
        throw std::runtime_error("Requested validations layers not supported.");
    }

    vk::ApplicationInfo appInfo{
        "Test App", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0
    };
    std::vector<const char*> extensions = getRequiredExtensions();

    vk::InstanceCreateInfo instanceInfo{};
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.setPEnabledExtensionNames(extensions);

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = getDebugMessengerCreateInfo();
    if (validationLayersEnabled) {
        instanceInfo.setPEnabledLayerNames(validationLayers);
        instanceInfo.pNext = &debugCreateInfo;
    }
    instance = vk::createInstanceUnique(instanceInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
}

std::vector<const char*> GraphicsDevice::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (validationLayersEnabled) {
        extensions.push_back("VK_EXT_debug_utils");
    }
    return extensions;
}

bool GraphicsDevice::checkValidationLayerSupport() {
    for (const auto& validationLayer : validationLayers) {
        bool foundLayer = false;
        for (const auto& availableLayer : vk::enumerateInstanceLayerProperties()) {
            if (strcmp(availableLayer.layerName, validationLayer)) {
                foundLayer = true;
                break;
            }
        }
        if (!foundLayer) return false;
    }
    return true;
}

vk::DebugUtilsMessengerCreateInfoEXT GraphicsDevice::getDebugMessengerCreateInfo() {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral 
    | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = [](
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) -> VKAPI_ATTR VkBool32 VKAPI_CALL {
        //Ignore error that is thrown as a false positive when resizing the application's window
        std::string ignoreError = "VUID-VkSwapchainCreateInfoKHR-imageExtent-01274";
        if (ignoreError != pCallbackData->pMessageIdName) {
            std::cerr << pCallbackData->pMessage << '\n';
        }
        return VK_FALSE;
    };
    return createInfo;
}

void GraphicsDevice::createSurface() {
    VkSurfaceKHR nativeSurface;
    window.createWindowSurface(*instance, &nativeSurface);
    vk::UniqueSurfaceKHR uniqueSurface{vk::UniqueSurfaceKHR{nativeSurface, *instance}};
    surface.swap(uniqueSurface);
}

void GraphicsDevice::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> physicalDevices = instance->enumeratePhysicalDevices();
    std::multimap<int, vk::PhysicalDevice> scoredDevices;

    if (physicalDevices.size() == 0) {
        throw std::runtime_error("Failed to find a GPU with Vulkan support!");
    }

    for (const auto& device : physicalDevices) {
        scoredDevices.insert(std::make_pair(rateDeviceSuitability(device), device));
    }

    if (scoredDevices.rbegin()->first > 0) {
        physicalDevice = scoredDevices.rbegin()->second;
    } else {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

int GraphicsDevice::rateDeviceSuitability(vk::PhysicalDevice device) {
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
    vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(device);

    int score = 0;
    if (!deviceFeatures.geometryShader) return 0;
    if (!queueFamilyIndices.isComplete()) return 0;
    if (!hasRequiredDeviceExtensions(device)) return 0;

    SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails(device);
    if (swapChainSupport.surfaceFormats.empty() || swapChainSupport.presentModes.empty()) return 0;

    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    }
    score += deviceProperties.limits.maxImageDimension2D;
    return score;
}

bool GraphicsDevice::hasRequiredDeviceExtensions(vk::PhysicalDevice device) {
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

QueueFamilyIndices GraphicsDevice::findQueueFamilyIndices(vk::PhysicalDevice device) {
    QueueFamilyIndices queueFamilyIndices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;
        }

        if (device.getSurfaceSupportKHR(i, *surface)) {
            queueFamilyIndices.presentFamily = i;
        }
        
        if (queueFamilyIndices.isComplete()) break;
    }
    return queueFamilyIndices;
}

void GraphicsDevice::createLogicalDevice() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value()
    };

    std::array<float, 1> queuePriorities = {1.0f};
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueInfos.push_back({{}, queueFamily, queuePriorities});
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.setQueueCreateInfos(queueInfos);
    deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);
    deviceCreateInfo.setPEnabledExtensionNames(requiredDeviceExtensions);

    if (validationLayersEnabled) {
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    device = physicalDevice.createDeviceUnique(deviceCreateInfo);
    graphicsQueue = device->getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
    presentQueue = device->getQueue(queueFamilyIndices.presentFamily.value(), 0);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
}

SwapChainSupportDetails GraphicsDevice::getSwapChainSupportDetails(vk::PhysicalDevice device) {
    SwapChainSupportDetails supportDetails;
    supportDetails.capabilities = device.getSurfaceCapabilitiesKHR(*surface);
    supportDetails.surfaceFormats = device.getSurfaceFormatsKHR(*surface);
    supportDetails.presentModes = device.getSurfacePresentModesKHR(*surface); 
    return supportDetails;
}

vk::Format GraphicsDevice::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format!");
}

void GraphicsDevice::createCommandPool() {
    commandPool = device->createCommandPoolUnique({
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient,
        getQueueFamilyIndices().graphicsFamily.value()
    });
}

uint32_t GraphicsDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}