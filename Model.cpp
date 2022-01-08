#include "Model.h"
#include "Utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "libs/tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {
    template <>
    struct hash<Model::Vertex> {
        size_t operator()(const Model::Vertex& vertex) const {
            size_t seed = 0;
            hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

Model::Model(GraphicsDevice &device, const Data& data) : graphicsDevice(device) {
    createVertexBuffers(data.vertices);
    createIndexBuffer(data.indices);
}

Model::~Model() {
    vkDestroyBuffer(graphicsDevice.getDevice(), vertexBuffer, nullptr);
    vkFreeMemory(graphicsDevice.getDevice(), vertexBufferMemory, nullptr);

    if (hasIndexBuffer) {
        vkDestroyBuffer(graphicsDevice.getDevice(), indexBuffer, nullptr);
        vkFreeMemory(graphicsDevice.getDevice(), indexBufferMemory, nullptr);
    }
}

std::unique_ptr<Model> Model::createModelFromFile(GraphicsDevice& device, const std::string& filepath) {
    Data data{};
    data.loadModel(filepath);
    return std::make_unique<Model>(device, data);
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
    vertexCount = vertices.size();
    assert(vertexCount >= 3 && "Number of vertices must be greater than or equal to 3!");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    graphicsDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(graphicsDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(graphicsDevice.getDevice(), stagingBufferMemory);

    graphicsDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory);
    graphicsDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(graphicsDevice.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(graphicsDevice.getDevice(), stagingBufferMemory, nullptr);
}

void Model::createIndexBuffer(const std::vector<uint32_t>& indices) {
    indexCount = indices.size();
    hasIndexBuffer = indexCount > 0;
    if (!hasIndexBuffer) return;
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    graphicsDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(graphicsDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(graphicsDevice.getDevice(), stagingBufferMemory);

    graphicsDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexBufferMemory);
    graphicsDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(graphicsDevice.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(graphicsDevice.getDevice(), stagingBufferMemory, nullptr);
}

void Model::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void Model::draw(VkCommandBuffer commandBuffer) {
    if (hasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    //position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);
    //color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}

void Model::Data::loadModel(const std::string& filepath) {
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filepath)) {
        if (!reader.Error().empty()) {
            throw std::runtime_error(reader.Error());
        }
    }
    if (!reader.Warning().empty()) {
        std::cout << reader.Warning() << '\n';
    }

    auto& attrib = reader.GetAttrib();
    auto& faces = reader.GetShapes();
    //auto& materials = reader.GetMaterials();

    vertices.clear();
    indices.clear();
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& face : faces) {
        for (const auto& vertexIndices : face.mesh.indices) {
            Vertex vertex{};
            vertex.position = {
                attrib.vertices[3 * vertexIndices.vertex_index + 0],
                attrib.vertices[3 * vertexIndices.vertex_index + 1],
                attrib.vertices[3 * vertexIndices.vertex_index + 2],
            };
            vertex.color = {
                attrib.colors[3 * vertexIndices.vertex_index + 0],
                attrib.colors[3 * vertexIndices.vertex_index + 1],
                attrib.colors[3 * vertexIndices.vertex_index + 2],
            };
            if (vertexIndices.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * vertexIndices.normal_index + 0],
                    attrib.normals[3 * vertexIndices.normal_index + 1],
                    attrib.normals[3 * vertexIndices.normal_index + 2],
                };
            }
            if (vertexIndices.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * vertexIndices.texcoord_index + 0],
                    attrib.texcoords[2 * vertexIndices.texcoord_index + 1],
                };
            }

            if (uniqueVertices.count(vertex) == 0) {
                vertices.push_back(vertex);
                uniqueVertices[vertex] = vertices.size() - 1;
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}