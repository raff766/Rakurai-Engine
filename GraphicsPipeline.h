#pragma once

#include "GraphicsDevice.h"
#include <string>
#include <vector>
#include <utility>

struct PipelineConfigInfo {
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
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
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    void operator=(const GraphicsPipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    
    static PipelineConfigInfo getDefaultPipelineConfigInfo(uint32_t width, uint32_t height);

private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo);

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    GraphicsDevice& graphicsDevice;
    VkPipeline graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};