#version 450

layout(location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

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
    outColor = vec4(fragColor, 1.0);
}