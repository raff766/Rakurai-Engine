#pragma once

#include "GraphicsDevice.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>

namespace rkrai {
class SwapChain {
    public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(GraphicsDevice& graphicsDevice, vk::Extent2D windowExtent);
    SwapChain(GraphicsDevice& graphicsDevice, vk::Extent2D windowExtent, std::shared_ptr<SwapChain> previous);
    SwapChain(const SwapChain&) = delete;
    void operator=(const SwapChain&) = delete;

    vk::ResultValue<uint32_t> acquireNextImage();
    void submitDrawCommands(const vk::CommandBuffer& buffer);
    vk::Result presentImage(uint32_t imageIndex);

    bool compareSwapChainFormats(const SwapChain& swapChain) const {
        return swapChain.swapChainImageFormat == swapChainImageFormat && swapChain.swapChainDepthFormat == swapChainDepthFormat;
    }
    vk::Extent2D getExtent() { return swapChainExtent; }
    float getAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
    vk::RenderPass getRenderPass() { return *renderPass; }
    size_t getImageCount() { return swapChainImages.size(); }
    vk::Framebuffer getFrameBuffer(int index) { return *swapChainFramebuffers[index]; }

    private:
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::UniqueImageView> swapChainImageViews;
    std::vector<vk::UniqueFramebuffer> swapChainFramebuffers;
    
    GraphicsDevice& graphicsDevice;
    vk::Extent2D windowExtent;

    vk::UniqueSwapchainKHR swapChain;
    std::shared_ptr<SwapChain> oldSwapChain;
    vk::Format swapChainImageFormat;
    vk::Format swapChainDepthFormat;
    vk::Extent2D swapChainExtent;

    vk::UniqueRenderPass renderPass;

    std::vector<vk::UniqueImage> depthImages;
    std::vector<vk::UniqueDeviceMemory> depthImageMemories;
    std::vector<vk::UniqueImageView> depthImageViews;

    //Synchronization objects
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFrameFences;
    std::vector<vk::Fence> imagesInFlight;
    int currentFrame = 0;

    bool hasStencilComponent(vk::Format format) { return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint; }
    
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    vk::Format findDepthFormat();
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();
};
}