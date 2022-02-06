#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <cassert>
#include <limits>

namespace rkrai {
void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
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

//By multiplying the inverse of the camera's translation with the transpose of the camera's
//rotation, we can move the camera to the center of vulkan's view volume's near plane
//and then undo the camera's rotation to make it face the positive z direction.
//This is necessary in order to move all of the objects the camera is able to see
//into vulkan's view volume to be displayed.
void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 i{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 j{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 k{(c2 * s1), (-s2), (c1 * c2)};
    viewMatrix = glm::mat4{1.0f};
    viewMatrix[0][0] = i.x;
    viewMatrix[1][0] = i.y;
    viewMatrix[2][0] = i.z;
    viewMatrix[0][1] = j.x;
    viewMatrix[1][1] = j.y;
    viewMatrix[2][1] = j.z;
    viewMatrix[0][2] = k.x;
    viewMatrix[1][2] = k.y;
    viewMatrix[2][2] = k.z;
    viewMatrix[3][0] = -glm::dot(i, position);
    viewMatrix[3][1] = -glm::dot(j, position);
    viewMatrix[3][2] = -glm::dot(k, position);
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    const glm::vec3 k{glm::normalize(direction)};
    const glm::vec3 i{glm::normalize(glm::cross(k, up))};
    const glm::vec3 j{glm::cross(k, i)};

    viewMatrix = glm::mat4{1.0f};
    viewMatrix[0][0] = i.x;
    viewMatrix[1][0] = i.y;
    viewMatrix[2][0] = i.z;
    viewMatrix[0][1] = j.x;
    viewMatrix[1][1] = j.y;
    viewMatrix[2][1] = j.z;
    viewMatrix[0][2] = k.x;
    viewMatrix[1][2] = k.y;
    viewMatrix[2][2] = k.z;
    viewMatrix[3][0] = -glm::dot(i, position);
    viewMatrix[3][1] = -glm::dot(j, position);
    viewMatrix[3][2] = -glm::dot(k, position);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    setViewDirection(position, target - position, up);
}
}