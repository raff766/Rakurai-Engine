#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragNormalWorldDir;

layout(push_constant) uniform Push {
    mat4 modelMat;
    mat4 normalMat;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 projMat;
    mat4 viewMat;
    vec4 ambientLightColor;
    vec4 lightColor;
    vec3 lightPosition;
} ubo;

void main() {
    vec4 vertexWorldPos = push.modelMat * vec4(position, 1.0);
    vec3 normalWorldDir = normalize(mat3(push.normalMat) * normal);

    gl_Position = ubo.projMat * ubo.viewMat * vertexWorldPos;

    fragColor = color;
    fragWorldPos = vertexWorldPos.xyz;
    fragNormalWorldDir = normalWorldDir;
}