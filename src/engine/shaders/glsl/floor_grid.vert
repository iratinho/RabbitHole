#version 450

layout(set=0, binding=0) uniform SceneData {
    mat4 view;
    mat4 proj;
} data;

layout(location = 0) in vec3 in_vertex_position;

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;

// Grid position are in clipped space
vec3 gridPlane[6] = vec3[] (
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;

    nearPoint = UnprojectPoint(in_vertex_position.x, in_vertex_position.y, 0.0, data.view, data.proj).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(in_vertex_position.x, in_vertex_position.y, 1.0, data.view, data.proj).xyz; // unprojecting on the far plane
    gl_Position = vec4(in_vertex_position.xy, 0.0f, 1.0); // using directly the clipped coordinates
}