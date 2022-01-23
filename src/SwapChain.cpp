#include "SwapChain.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace rkrai {
SwapChain::SwapChain(GraphicsDevice& graphicsDevice, vk::Extent2D windowExtent) 
    :  graphicsDevice(graphicsDevice), windowExtent(windowExtent) {
    init();
}

SwapChain::SwapChain(GraphicsDevice& graphicsDevice, vk::Extent2D windowExtent, std::shared_ptr<SwapChain> previous) 
    :  graphicsDevice(graphicsDevice), windowExtent(windowExtent), oldSwapChain(previous) {
    init();
    oldSwapChain = nullptr; //Clean up old swap chain
}

void SwapChain::init() {
    createSwapChain();
    createImageViews();
    createDepthResources();
    createRenderPass();
    createFramebuffers();
    createSyncObjects();
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = windowExtent;
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

vk::Format SwapChain::findDepthFormat() {
    return graphicsDevice.findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

void SwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = graphicsDevice.getSwapChainSupportDetails();
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.surfaceFormats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.surface = graphicsDevice.getSurface();
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = extent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = graphicsDevice.getQueueFamilyIndices();
    std::array<u_int32_t, 2> queueFamilyIndices{indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        swapChainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainInfo.setQueueFamilyIndices(queueFamilyIndices);
    } else {
        swapChainInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapChainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : *oldSwapChain->swapChain;

    swapChain = graphicsDevice.getDevice().createSwapchainKHRUnique(swapChainInfo);
    swapChainImages = graphicsDevice.getDevice().getSwapchainImagesKHR(*swapChain);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (int i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = graphicsDevice.getDevice().createImageViewUnique({
            {}, swapChainImages[i], vk::ImageViewType::e2D, swapChainImageFormat, {}, 
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        });
    }
}

void SwapChain::createDepthResources() {
    swapChainDepthFormat = findDepthFormat();
    depthImages.resize(getImageCount());
    depthImageMemories.resize(getImageCount());
    depthImageViews.resize(getImageCount());

    for (int i = 0; i < depthImages.size(); i++) {
        vk::ImageCreateInfo depthImageInfo{};
        depthImageInfo.imageType = vk::ImageType::e2D;
        depthImageInfo.extent.width = swapChainExtent.width;
        depthImageInfo.extent.height = swapChainExtent.height;
        depthImageInfo.extent.depth = 1;
        depthImageInfo.mipLevels = 1;
        depthImageInfo.arrayLayers = 1;
        depthImageInfo.format = swapChainDepthFormat;
        depthImageInfo.tiling = vk::ImageTiling::eOptimal;
        depthImageInfo.initialLayout = vk::ImageLayout::eUndefined;
        depthImageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        depthImageInfo.samples = vk::SampleCountFlagBits::e1;
        depthImageInfo.sharingMode = vk::SharingMode::eExclusive;

        depthImages[i] = graphicsDevice.getDevice().createImageUnique(depthImageInfo);
        auto memRequirements = graphicsDevice.getDevice().getImageMemoryRequirements(*depthImages[i]);

        depthImageMemories[i] = graphicsDevice.getDevice().allocateMemoryUnique({
            memRequirements.size,
            graphicsDevice.findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
        });
        graphicsDevice.getDevice().bindImageMemory(*depthImages[i], *depthImageMemories[i], 0);

        depthImageViews[i] = graphicsDevice.getDevice().createImageViewUnique({
            {}, *depthImages[i], vk::ImageViewType::e2D, swapChainDepthFormat, {},
            {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}
        });
    }
}

void SwapChain::createRenderPass() {
    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = swapChainDepthFormat;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.setColorAttachments(colorAttachmentRef);
    subpass.setPDepthStencilAttachment(&depthAttachmentRef);

    vk::SubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcAccessMask = {};
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    std::array<vk::AttachmentDescription, 2> attachments{colorAttachment, depthAttachment};

    renderPass = graphicsDevice.getDevice().createRenderPassUnique({{}, attachments, subpass, dependency});
}

void SwapChain::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (int i = 0; i < swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 2> attachments{ *swapChainImageViews[i], *depthImageViews[i] };
        swapChainFramebuffers[i] = graphicsDevice.getDevice().createFramebufferUnique({
            {}, *renderPass, attachments, swapChainExtent.width, swapChainExtent.height, 1
        });
    }
}

void SwapChain::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFrameFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), {});

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = graphicsDevice.getDevice().createSemaphoreUnique({});
        renderFinishedSemaphores[i] = graphicsDevice.getDevice().createSemaphoreUnique({});
        inFlightFrameFences[i] = graphicsDevice.getDevice().createFenceUnique({vk::FenceCreateFlagBits::eSignaled});
    }
}

vk::ResultValue<uint32_t> SwapChain::acquireNextImage() {
    graphicsDevice.getDevice().waitForFences(*inFlightFrameFences[currentFrame], VK_TRUE, UINT64_MAX);
    try {
        vk::ResultValue<uint32_t> result = graphicsDevice.getDevice().acquireNextImageKHR(
            *swapChain, UINT64_MAX, *imageAvailableSemaphores[currentFrame]
        );

        uint32_t imageIndex = result.value;
        if (imagesInFlight[imageIndex]) {
            graphicsDevice.getDevice().waitForFences(imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = *inFlightFrameFences[currentFrame];

        return result;
    } catch (const vk::OutOfDateKHRError& error) {
        return {vk::Result::eErrorOutOfDateKHR, UINT32_MAX};
    }
}

void SwapChain::submitDrawCommands(const vk::CommandBuffer& buffer) {
    std::vector<vk::Semaphore> waitSemaphores = { *imageAvailableSemaphores[currentFrame] };
    std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    std::vector<vk::Semaphore> signalSemaphores = { *renderFinishedSemaphores[currentFrame] };

    vk::SubmitInfo submitInfo{waitSemaphores, waitStages, buffer, signalSemaphores};

    graphicsDevice.getDevice().resetFences(*inFlightFrameFences[currentFrame]);
    graphicsDevice.getGraphicsQueue().submit(submitInfo, *inFlightFrameFences[currentFrame]);
}

vk::Result SwapChain::presentImage(uint32_t imageIndex) {
    vk::PresentInfoKHR presentInfo{*renderFinishedSemaphores[currentFrame], *swapChain, imageIndex};
    
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    try {
        return graphicsDevice.getPresentQueue().presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError& error) {
        return vk::Result::eErrorOutOfDateKHR;
    }
}
}