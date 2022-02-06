#version 450

layout(location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    vec4 billboardColor;
    vec2 billboardDimensions;
    vec3 billboardPosition;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 projMat;
    mat4 viewMat;
} ubo;

void main() {
    float distance = sqrt(dot(fragOffset, fragOffset));
    if (distance >= 1.0) discard;
    outColor = vec4(push.billboardColor.xyz * push.billboardColor.w, 1.0);
}