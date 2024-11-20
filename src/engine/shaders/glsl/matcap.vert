#version 450

layout(set=0, binding=0) uniform PerModelData {
    mat4 modelViewMatrix;
    mat4 mvpMatrix;
    mat4 normalMatrix;
} perModelData;

// Inputs
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

// Outputs
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 e;

void main() {
    // Convert vertexPosition to a vec4 with w = 1.0 for matrix multiplication
    e = normalize((perModelData.modelViewMatrix * vec4(vertexPosition, 1.0)).xyz);
    
    // Convert vertexNormal to a vec4 with w = 0.0 for correct normal transformation
    fragNormal = normalize((perModelData.normalMatrix * vec4(vertexNormal, 0.0)).xyz);

    // Calculate gl_Position with vertexPosition as a vec4 with w = 1.0
    gl_Position = perModelData.mvpMatrix * vec4(vertexPosition, 1.0);
}