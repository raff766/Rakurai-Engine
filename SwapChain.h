#pragma once

#include "GraphicsDevice.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

class SwapChain {
    public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(GraphicsDevice& graphicsDevice, VkExtent2D windowExtent);
    SwapChain(GraphicsDevice& graphicsDevice, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();

    VkResult acquireNextImage(uint32_t& imageIndex);
    VkResult submitDrawCommands(const VkCommandBuffer* buffer);
    VkResult presentImage(uint32_t imageIndex);

    bool compareSwapChainFormats(const SwapChain& swapChain) const {
        return swapChain.swapChainImageFormat == swapChainImageFormat /*&& swapChain.swapChainDepthFormat == swapChainDepthFormat*/;
    }
    VkExtent2D getExtent() { return swapChainExtent; }
    VkRenderPass getRenderPass() { return renderPass; }
    size_t getImageCount() { return swapChainImages.size(); }
    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }

    private:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    GraphicsDevice& graphicsDevice;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::shared_ptr<SwapChain> oldSwapChain;
    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    VkRenderPass renderPass;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemories;
    std::vector<VkImageView> depthImageViews;

    //Synchronization objects
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFrameFences;
    std::vector<VkFence> imagesInFlight;
    int currentFrame = 0;

    bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkFormat findDepthFormat();
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();
};