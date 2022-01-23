#include "GraphicsPipeline.h"
#include "Model.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <cassert>

namespace rkrai {
PipelineConfigInfo::PipelineConfigInfo(const PipelineConfigInfo& other) 
    : viewportInfo(other.viewportInfo), inputAssemblyInfo(other.inputAssemblyInfo),
    rasterizationInfo(other.rasterizationInfo), multisampleInfo(other.multisampleInfo),
    colorBlendAttachment(other.colorBlendAttachment), colorBlendInfo(other.colorBlendInfo),
    depthStencilInfo(other.depthStencilInfo), dynamicStateEnables(other.dynamicStateEnables),
    dynamicStateInfo(other.dynamicStateInfo), pipelineLayout(other.pipelineLayout),
    renderPass(other.renderPass), subpass(other.subpass) {
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
}
PipelineConfigInfo& PipelineConfigInfo::operator=(const PipelineConfigInfo& other) {
    viewportInfo = other.viewportInfo;
    inputAssemblyInfo = other.inputAssemblyInfo;
    rasterizationInfo = other.rasterizationInfo;
    multisampleInfo = other.multisampleInfo;
    colorBlendAttachment = other.colorBlendAttachment;
    colorBlendInfo = other.colorBlendInfo;
    depthStencilInfo = other.depthStencilInfo;
    dynamicStateEnables = other.dynamicStateEnables;
    dynamicStateInfo = other.dynamicStateInfo;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    subpass = other.subpass;

    colorBlendInfo.pAttachments = &colorBlendAttachment;
    dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
    return *this;
}

GraphicsPipeline::GraphicsPipeline(
    GraphicsDevice &graphicsDevice, 
    const std::string& vertFilepath, 
    const std::string& fragFilepath, 
    const PipelineConfigInfo& configInfo) : graphicsDevice(graphicsDevice) {
    createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

std::vector<char> GraphicsPipeline::readFile(const std::string& filepath) {
    std::ifstream file{filepath, std::ios::binary};

    size_t fileSize = std::filesystem::file_size(filepath);
    std::vector<char> buffer(fileSize);

    file.read(buffer.data(), fileSize);
    if (!file.good()) {
        std::runtime_error("Failed to open file: " + filepath);
    }
    file.close();

    return buffer;
}

void GraphicsPipeline::createGraphicsPipeline(
    const std::string& vertFilepath,
    const std::string& fragFilepath,
    const PipelineConfigInfo& configInfo) {
    
    //assert(!configInfo.pipelineLayout && "Cannot create graphics pipeline: no pipelineLayout provided!");
    //assert(!configInfo.renderPass && "Cannot create graphics pipeline: no renderPass provided!");
    
    std::vector<char> vertCode = readFile(vertFilepath);
    std::vector<char> fragCode = readFile(fragFilepath);

    vertShaderModule = createShaderModule(vertCode);
    fragShaderModule = createShaderModule(fragCode);

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main"},
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main"}
    };

    auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
    auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{{}, bindingDescriptions, attributeDescriptions};

    /*vk::GraphicsPipelineCreateInfo pipelineInfo{
        {}, shaderStages, vertexInputInfo, configInfo.inputAssemblyInfo, {}, configInfo.viewportInfo,
        configInfo.rasterizationInfo, configInfo.multisampleInfo, configInfo.depthStencilInfo,
        configInfo.colorBlendInfo, configInfo.dynamicStateInfo, 
        configInfo.pipelineLayout, configInfo.renderPass, configInfo.subpass, {}, -1
    };*/
    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.setStages(shaderStages);
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.renderPass = configInfo.renderPass;
    pipelineInfo.subpass = configInfo.subpass;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    graphicsPipeline = graphicsDevice.getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;
}

vk::UniqueShaderModule GraphicsPipeline::createShaderModule(const std::vector<char>& code) {
    return graphicsDevice.getDevice().createShaderModuleUnique({
        {}, code.size(), reinterpret_cast<const uint32_t*>(code.data())
    });
}

void GraphicsPipeline::bind(vk::CommandBuffer commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
}

PipelineConfigInfo GraphicsPipeline::getDefaultPipelineConfigInfo() {
    PipelineConfigInfo pipelineInfo{};
    pipelineInfo.inputAssemblyInfo = {{}, vk::PrimitiveTopology::eTriangleList};
    pipelineInfo.viewportInfo = {{}, 1, {}, 1, {}};
    pipelineInfo.rasterizationInfo = {
        {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, VK_FALSE, 0, 0, 0, 1.0f
    };
    pipelineInfo.multisampleInfo = {{}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f};
    pipelineInfo.colorBlendAttachment = {
        VK_FALSE,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    pipelineInfo.colorBlendInfo = {{}, VK_FALSE, vk::LogicOp::eCopy, pipelineInfo.colorBlendAttachment, {}};
    pipelineInfo.depthStencilInfo = {{}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1.0f};
    pipelineInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    pipelineInfo.dynamicStateInfo = {{}, pipelineInfo.dynamicStateEnables};

    return pipelineInfo;
}
}