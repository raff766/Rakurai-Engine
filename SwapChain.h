#pragma once

#include "GraphicsDevice.h"

#include <vulkan/vulkan.h>
#include <vector>

class SwapChain {
    public:
    SwapChain(GraphicsDevice& graphicsDevice, VkExtent2D windowExtent);
    ~SwapChain();

    VkExtent2D getExtent() { return swapChainExtent; };
    VkRenderPass getRenderPass() { return renderPass; };
    size_t getImageCount() { return swapChainImages.size(); };
    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }

    uint32_t acquireNextImage();
    void submitDrawCommands(const VkCommandBuffer* buffer);
    VkResult presentImage(uint32_t imageIndex);

    private:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    GraphicsDevice& graphicsDevice;
    VkExtent2D windowExtent;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;

    //Synchronization objects
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFrameFences;
    std::vector<VkFence> imagesInFlight;
    int currentFrame = 0;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();
};