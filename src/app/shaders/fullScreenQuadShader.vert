#version 450

const vec2 vertices[6] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),

    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
};

const vec2 coords[6] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),

    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
};

layout(location = 0) out vec2 texCoords;

void main()
{
    vec2 pos = vertices[gl_VertexIndex];
    vec2 ndc = vec2((pos.x * 2.0) -1.0, (pos.y * 2.0) - 1.0);
    texCoords = coords[gl_VertexIndex];
    gl_Position = vec4(ndc, 0.0, 1.0);
}