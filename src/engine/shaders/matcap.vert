#version 450

struct Matrix {
    mat4 mvp_matrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 normalMatrix;
};

// Push Constants
layout(push_constant) uniform PushConstants {
    Matrix matrix;
} pushConstant;

// Inputs
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

// Outputs
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 e;


void main() {
    mat4 modelViewMatrix = pushConstant.matrix.viewMatrix * pushConstant.matrix.modelMatrix;
    mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));

    e = normalize( vec3( modelViewMatrix * vec4( vertexPosition, 1.0 ) ) );
    fragNormal = normalize( normalMatrix * vertexNormal ).xyz;

    gl_Position = pushConstant.matrix.mvp_matrix * vec4(vertexPosition, 1.0);
}