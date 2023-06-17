#version 450

struct DirectionalLight {
    vec4 color;
    vec4 position;
    float intensity;
};

layout(std430, push_constant) uniform PushConstants {
    mat4 mvp_matrix;
    DirectionalLight directionalLight;
} push_constants;

layout(location = 0) in vec3 in_vertex_position;
layout(location = 1) in vec3 in_vertex_normal;

layout(location = 0) out vec3 light_color;
layout(location = 1) out float light_intensity;
layout(location = 2) out vec3 light_position;
layout(location = 3) out vec3 vertex_normal;
layout(location = 4) out vec3 fragPos;

void main()
{
    light_intensity = push_constants.directionalLight.intensity;
    light_color = push_constants.directionalLight.color.rgb.xyz;
    light_position = push_constants.directionalLight.position.xyz;
    vertex_normal = in_vertex_normal;

    fragPos = vec3(in_vertex_position);

    // using last arg as 1.0 so that the normalization wont happen
    gl_Position = push_constants.mvp_matrix * vec4(in_vertex_position, 1.0);
}