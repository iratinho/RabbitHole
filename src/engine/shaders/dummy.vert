#version 450

struct DirectionalLight {
    vec4 color;
    vec4 position;
    float intensity;
};

struct CameraData {
    vec3 cameraPosition;
};

layout(std430, push_constant) uniform PushConstants {
    mat4 mvp_matrix;
    DirectionalLight directionalLight;
    CameraData cameraData;
} push_constants;

layout(location = 0) in vec3 in_vertex_position;
layout(location = 1) in vec3 in_vertex_normal;

layout(location = 0) out vec3 lightColor;
layout(location = 1) out float lightIntensity;
layout(location = 2) out vec3 lightPosition;
layout(location = 3) out vec3 vertexNormal;
layout(location = 4) out vec3 fragPos;
layout(location = 5) out vec3 cameraPosition;

void main()
{
    lightIntensity = push_constants.directionalLight.intensity;
    lightColor = push_constants.directionalLight.color.rgb.xyz;
    lightPosition = push_constants.directionalLight.position.xyz;
    vertexNormal = in_vertex_normal;
    fragPos = vec3(in_vertex_position);
    cameraPosition = push_constants.cameraData.cameraPosition;

    // using last arg as 1.0 so that the normalization wont happen
    gl_Position = push_constants.mvp_matrix * vec4(in_vertex_position, 1.0);
}