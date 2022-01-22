#pragma once

#include "GraphicsDevice.h"
#include "GraphicsBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <optional>

class Model {
    public:
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };
    struct Data {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void loadModel(const std::string& filepath);
    };

    Model(GraphicsDevice& device, const Data& data);
    Model(GraphicsDevice& device, const std::string& filepath);
    Model(const Model&) = delete;
    void operator=(const Model&) = delete;

    void bind(vk::CommandBuffer commandBuffer);
    void draw(vk::CommandBuffer commandBuffer);

    private:
    GraphicsDevice& graphicsDevice;

    std::optional<GraphicsBuffer> vertexBuffer;
    uint32_t vertexCount;

    bool hasIndexBuffer = false;
    std::optional<GraphicsBuffer> indexBuffer;
    uint32_t indexCount;

    void createVertexBuffers(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<uint32_t>& indices);
};