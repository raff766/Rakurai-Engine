#pragma once

#include "Model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace rkrai {
struct TransformComponent {
    glm::vec3 translation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

    glm::mat4 modelMatrix();
    glm::mat3 normalMatrix();
};

class GameObject {
    public:
    using id_t = unsigned int;

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

    GameObject(const GameObject&) = delete;
    void operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;

    static GameObject createGameObject() {
        static id_t currentId = 0;
        return GameObject{currentId++};
    }

    id_t getId() { return id; }

    private:
    id_t id;

    GameObject(id_t objId) : id(objId) {}
};
}