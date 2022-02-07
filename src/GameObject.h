#pragma once

#include "Model.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace rkrai {
struct TransformComponent {
    glm::vec3 translation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

    glm::mat4 modelMatrix() const;
    glm::mat3 normalMatrix() const;
};

struct BillboardComponent {
    glm::vec4 color{1.0f}; //w is intensity
    glm::vec2 dimensions{0.1f};
};

struct PointLightComponent {
    glm::vec4 color{1.0f}; //w is intensity
};

class GameObject {
    public:
    using id_t = unsigned int;

    TransformComponent transform{};
    glm::vec3 color{};

    std::shared_ptr<Model> model;
    std::shared_ptr<BillboardComponent> billboard;
    std::shared_ptr<PointLightComponent> pointLight;

    GameObject() {
        static id_t currentId = 0;
        id = currentId++;
    }
    GameObject(const GameObject&) = delete;
    void operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;

    id_t getId() { return id; }

    private:
    id_t id;

    GameObject(id_t objId) : id(objId) {}
};
}