#pragma once

#include "GraphicsDevice.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include <utility>

namespace rkrai {
struct PipelineConfigInfo {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    vk::PipelineViewportStateCreateInfo viewportInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<vk::DynamicState> dynamicStateEnables;
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
    vk::PipelineLayout pipelineLayout = nullptr;
    vk::RenderPass renderPass = nullptr;
    uint32_t subpass = 0;

    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo& other);
    PipelineConfigInfo& operator=(const PipelineConfigInfo& other);
};
class GraphicsPipeline {
public:
    GraphicsPipeline(
        GraphicsDevice &graphicsDevice, 
        const std::string& vertFilepath, 
        const std::string& fragFilepath, 
        const PipelineConfigInfo& configInfo);
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    void operator=(const GraphicsPipeline&) = delete;
    GraphicsPipeline(GraphicsPipeline&&) = default;
    GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

    void bind(vk::CommandBuffer commandBuffer);
    
    static PipelineConfigInfo getDefaultPipelineConfigInfo();

private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo);

    vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);

    GraphicsDevice& graphicsDevice;
    vk::UniquePipeline graphicsPipeline;
    vk::UniqueShaderModule vertShaderModule;
    vk::UniqueShaderModule fragShaderModule;
};
}