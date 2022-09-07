#include "Model.h"
#include "Utils.h"
#include "GraphicsBuffer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {
    template <>
    struct hash<rkrai::Model::Vertex> {
        size_t operator()(const rkrai::Model::Vertex& vertex) const {
            size_t seed = 0;
            rkrai::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace rkrai {
Model::Model(GraphicsDevice& device, const Data& data) : graphicsDevice(device) {
    createVertexBuffers(data.vertices);
    createIndexBuffer(data.indices);
}

Model::Model(GraphicsDevice& device, const std::string& filepath) : graphicsDevice(device) {
    Data data{};
    data.loadModel(filepath);
    createVertexBuffers(data.vertices);
    createIndexBuffer(data.indices);
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
    vertexCount = vertices.size();
    assert(vertexCount >= 3 && "Number of vertices must be greater than or equal to 3!");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    GraphicsBuffer stagingBuffer(
        graphicsDevice,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    stagingBuffer.mapData(vertices.data());

    vertexBuffer.emplace(
        graphicsDevice,
        bufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    vertexBuffer->copyFrom(stagingBuffer);
}

void Model::createIndexBuffer(const std::vector<uint32_t>& indices) {
    indexCount = indices.size();
    hasIndexBuffer = indexCount > 0;
    if (!hasIndexBuffer) return;
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

    GraphicsBuffer stagingBuffer{
        graphicsDevice,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };

    stagingBuffer.mapData(indices.data());

    indexBuffer.emplace(
        graphicsDevice,
        bufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    indexBuffer->copyFrom(stagingBuffer);
}

void Model::bind(vk::CommandBuffer commandBuffer) {
    commandBuffer.bindVertexBuffers(0, vertexBuffer->getBuffer(), {0});
    if (hasIndexBuffer) {
        commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
    }
}

void Model::draw(vk::CommandBuffer commandBuffer) {
    if (hasIndexBuffer) {
        commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
    } else {
        commandBuffer.draw(vertexCount, 1, 0, 0);
    }
}

std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    return {{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};
}

std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
        {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)},
        {2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
        {3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)}
    };
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
                    1.0f - attrib.texcoords[2 * vertexIndices.texcoord_index + 1]
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
}