#version 450

layout(location = 0) out vec3 vertex_color;

vec2 vertex_pos[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 vertex_colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main()
{
    // using last arg as 1.0 so that the normalization wont happen
    gl_Position = vec4(vertex_pos[gl_VertexIndex], 0.0, 1.0);
    vertex_color = vertex_colors[gl_VertexIndex];
}