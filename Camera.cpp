#include "Camera.h"

#include <cassert>
#include <limits>

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
    /*float scale = 20;
    float viewVolumeWidth = (aspect * scale) / 2;
    float viewVolumeHeight = scale / 2;*/
    projectionMatrix = glm::mat4{1.0f};
    projectionMatrix[0][0] = 2.0f / (right - left);
    projectionMatrix[1][1] = 2.0f / (bottom - top);
    projectionMatrix[2][2] = 1.0f / (far - near);
    projectionMatrix[3][0] = -(left + right) / (right - left);
    projectionMatrix[3][1] = -(top + bottom) / (bottom - top);
    projectionMatrix[3][2] = -near / (far - near);
}
 
void Camera::setPerspectiveProjection(float fovY, float aspect, float near, float far) {
    projectionMatrix = glm::mat4{0.0f};
    float inverseTanFovY = 1 / glm::tan(glm::radians(fovY / 2));
    projectionMatrix[0][0] = inverseTanFovY;
    projectionMatrix[1][1] = aspect * inverseTanFovY;
    projectionMatrix[2][2] = far / (far - near);
    projectionMatrix[2][3] = 1;
    projectionMatrix[3][2] = -(far * near) / (far - near); 
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    const glm::vec3 w{glm::normalize(direction)};
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    const glm::vec3 v{glm::cross(w, u)};

    viewMatrix = glm::mat4{1.f};
    viewMatrix[0][0] = u.x;
    viewMatrix[1][0] = u.y;
    viewMatrix[2][0] = u.z;
    viewMatrix[0][1] = v.x;
    viewMatrix[1][1] = v.y;
    viewMatrix[2][1] = v.z;
    viewMatrix[0][2] = w.x;
    viewMatrix[1][2] = w.y;
    viewMatrix[2][2] = w.z;
    viewMatrix[3][0] = -glm::dot(u, position);
    viewMatrix[3][1] = -glm::dot(v, position);
    viewMatrix[3][2] = -glm::dot(w, position);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    setViewDirection(position, target - position, up);
}

void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
    const float c3 = glm::cos(-rotation.z);
    const float s3 = glm::sin(-rotation.z);
    const float c2 = glm::cos(-rotation.x);
    const float s2 = glm::sin(-rotation.x);
    const float c1 = glm::cos(-rotation.y);
    const float s1 = glm::sin(-rotation.y);
    viewMatrix = glm::mat4{
        {
            (c1 * c3 + s1 * s2 * s3),
            (c2 * s3),
            (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            (c3 * s1 * s2 - c1 * s3),
            (c2 * c3),
            (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            (c2 * s1),
            (-s2),
            (c1 * c2),
            0.0f,
        },
        {
            -position.x,
            -position.y,
            -position.z,
            1.0f
        }
    };
}