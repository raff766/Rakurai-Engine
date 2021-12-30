#include "GraphicsPipeline.h"
#include "Model.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <cassert>

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

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyShaderModule(graphicsDevice.getDevice(), vertShaderModule, nullptr);
    vkDestroyShaderModule(graphicsDevice.getDevice(), fragShaderModule, nullptr);
    vkDestroyPipeline(graphicsDevice.getDevice(), graphicsPipeline, nullptr);
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
    
    assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided!");
    assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided!");
    
    std::vector<char> vertCode = readFile(vertFilepath);
    std::vector<char> fragCode = readFile(fragFilepath);

    createShaderModule(vertCode, &vertShaderModule);
    createShaderModule(fragCode, &fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
    auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
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

    if (vkCreateGraphicsPipelines(graphicsDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void GraphicsPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(graphicsDevice.getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
}

void GraphicsPipeline::bind(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

PipelineConfigInfo GraphicsPipeline::getDefaultPipelineConfigInfo() {
    PipelineConfigInfo pipelineInfo{};
    pipelineInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    pipelineInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineInfo.viewportInfo.viewportCount = 1;
    pipelineInfo.viewportInfo.pViewports = nullptr;
    pipelineInfo.viewportInfo.scissorCount = 1;
    pipelineInfo.viewportInfo.pScissors = nullptr;
    
    pipelineInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    pipelineInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    pipelineInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineInfo.rasterizationInfo.lineWidth = 1.0f;
    pipelineInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    pipelineInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    pipelineInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    pipelineInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
    
    pipelineInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
    pipelineInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
    pipelineInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    pipelineInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
    
    pipelineInfo.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    pipelineInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    pipelineInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    pipelineInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    pipelineInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    pipelineInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    pipelineInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    pipelineInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
    
    pipelineInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    pipelineInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    pipelineInfo.colorBlendInfo.attachmentCount = 1;
    pipelineInfo.colorBlendInfo.pAttachments = &pipelineInfo.colorBlendAttachment;
    pipelineInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    pipelineInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    pipelineInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    pipelineInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
    
    pipelineInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    pipelineInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    pipelineInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    pipelineInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    pipelineInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    pipelineInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    pipelineInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    pipelineInfo.depthStencilInfo.front = {};  // Optional
    pipelineInfo.depthStencilInfo.back = {};   // Optional

    pipelineInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    pipelineInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineInfo.dynamicStateInfo.pDynamicStates = pipelineInfo.dynamicStateEnables.data();
    pipelineInfo.dynamicStateInfo.dynamicStateCount = pipelineInfo.dynamicStateEnables.size();
    pipelineInfo.dynamicStateInfo.flags = 0;

    return pipelineInfo;
}