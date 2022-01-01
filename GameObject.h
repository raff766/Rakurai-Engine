#pragma once

#include "Model.h"

#include <memory>

struct Transform2dComponent {
    glm::vec2 translation{};
    glm::vec2 scale{1.0f, 1.0f};
    float rotation;

    glm::mat2 mat2() {
        const float cos = glm::cos(rotation);
        const float sin = glm::sin(rotation);
        glm::mat2 rotationMat{{cos, sin}, {-sin, cos}};

        glm::mat2 scaleMat{{scale.x, 0.0f},{0.0f, scale.y}};
        return rotationMat * scaleMat; 
    };
};

class GameObject {
    public:
    using id_t = unsigned int;

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    Transform2dComponent transform2d{};

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