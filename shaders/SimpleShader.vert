#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUv;

layout(push_constant) uniform Push {
    mat4 modelMat;
    mat4 normalMat;
} push;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 projMat;
    mat4 viewMat;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 vertexWorldPos = push.modelMat * vec4(position, 1.0);
    vec3 normalWorld = normalize(mat3(push.normalMat) * normal);

    gl_Position = ubo.projMat * ubo.viewMat * vertexWorldPos;

    fragColor = color;
    fragWorldPos = vertexWorldPos.xyz;
    fragNormalWorld = normalWorld;
    fragUv = uv;
}