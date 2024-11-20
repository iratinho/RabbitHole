#version 450

// layout(std430, push_constant) uniform PushConstants {
//     mat4 mvp_matrix;
// } push_constants;

layout(set=0, binding=0) uniform GeneralData {
    float intensity; // 4
    vec3 color; // 12
    vec3 direction; // 16
    vec3 cameraPosition; // 12
} generalData;

layout(set=1, binding=0) uniform PerModelData {
    mat4 mvp_matrix;
} perModelData;

layout(location = 0) in vec3 in_vertex_position;
layout(location = 1) in vec3 in_vertex_normal;
layout(location = 2) in vec2 coords;

layout(location = 0) out vec3 lightColor;
layout(location = 1) out float lightIntensity;
layout(location = 2) out vec3 lightDirection;
layout(location = 3) out vec3 vertexNormal;
layout(location = 4) out vec3 fragPos;
layout(location = 5) out vec3 cameraPosition;
layout(location = 6) out vec2 tCoords;

void main()
{
    vec4 finalPos = perModelData.mvp_matrix * vec4(in_vertex_position, 1.0);
    
    lightIntensity = generalData.intensity;
    lightColor = generalData.color;
    lightDirection = generalData.direction;
    vertexNormal = in_vertex_normal;
    fragPos = finalPos.xyz;
    cameraPosition = generalData.cameraPosition;
    tCoords = coords;

    // using last arg as 1.0 so that the normalization wont happen
    gl_Position = finalPos;
}