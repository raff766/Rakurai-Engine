#version 450

layout(location = 0) out vec2 fragVertexOffset;

layout(push_constant) uniform Push {
    vec4 billboardColor;
    vec2 billboardDimensions;
    vec3 billboardPosition;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 projMat;
    mat4 viewMat;
} ubo;

const vec2 VERTEX_OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

void main() {
    fragVertexOffset = VERTEX_OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorldDir = {ubo.viewMat[0][0], ubo.viewMat[1][0], ubo.viewMat[2][0]};
    vec3 cameraUpWorldDir = {ubo.viewMat[0][1], ubo.viewMat[1][1], ubo.viewMat[2][1]};

    vec3 vertexPosWorld = push.billboardPosition + ((fragVertexOffset.x * push.billboardDimensions.x) * cameraRightWorldDir)
                                            + ((fragVertexOffset.y * push.billboardDimensions.y) * cameraUpWorldDir);

    gl_Position = ubo.projMat * ubo.viewMat * vec4(vertexPosWorld, 1.0);
}