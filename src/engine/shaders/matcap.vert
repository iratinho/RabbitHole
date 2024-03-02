#version 450

// Push Constants
layout(push_constant) uniform PushConstants {
    mat4 mvp_matrix;
} pushConstant;

// Inputs
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

// Outputs
layout(location = 0) out vec3 fragNormal;

void main() {
    fragNormal = vertexNormal;
    gl_Position = pushConstant.mvp_matrix * vec4(vertexPosition, 1.0);
}