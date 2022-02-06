#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormalWorldDir;

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
    vec3 directionToLight = ubo.lightPosition - fragWorldPos;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); //distance squared

    vec3 lightColor = attenuation * ubo.lightColor.xyz * ubo.lightColor.w;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorldDir), normalize(directionToLight)), 0);

    outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}