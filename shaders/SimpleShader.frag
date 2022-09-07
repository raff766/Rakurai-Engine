#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

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
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 normalWorld = normalize(fragNormalWorld);
    for (int i = 0; i < ubo.numLights; i++) {
        vec3 directionToLight = ubo.pointLights[i].position.xyz - fragWorldPos;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); //distance squared
        vec3 lightColor = ubo.pointLights[i].color.xyz * ubo.pointLights[i].color.w;
        diffuseLight += attenuation * lightColor * max(dot(normalWorld, normalize(directionToLight)), 0);
    }
    outColor = texture(texSampler, fragUv);
}