#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 modelMat;
    mat4 normalMat;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 projMat;
    mat4 viewMat;
    vec3 lightDirection;
} ubo;

const float AMBIENT = 0.02;

void main() {
    debugPrintfEXT("Hello world!");

    gl_Position = ubo.projMat * ubo.viewMat * push.modelMat * vec4(position, 1.0);

    vec3 normalWorldSpace = normalize(mat3(push.normalMat) * normal);
    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.lightDirection), 0);

    fragColor = lightIntensity * color;
}