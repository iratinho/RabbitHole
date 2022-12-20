#version 450

layout(push_constant) uniform PushConstants {
    mat4 mvp_matrix;
} push_constants;

layout(location = 0) in vec3 in_vertex_position;
layout(location = 1) in vec3 in_vertex_color;

layout(location = 0) out vec3 vertex_color;

void main()
{
    // using last arg as 1.0 so that the normalization wont happen
    gl_Position = push_constants.mvp_matrix * vec4(in_vertex_position, 1.0);
    vertex_color = in_vertex_color;
}