#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;

layout(set=0, binding=0) uniform SceneData {
    mat4 fragView;
    mat4 fragProj;
} data;


vec4 grid(vec3 fragPos3D, float scale, float intensity, bool skip) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

    if(skip) {
        color.a = 0.0;
    }

    return color * intensity;
}

vec4 drawAxis(vec3 fragPos3D) { 
    vec2 coord = fragPos3D.xz * 1.3;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;

    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);

    vec4 color = vec4(0, 0, 0, 0);

    // z axis
    if(fragPos3D.x > -1.0 * minimumx && fragPos3D.x < 1.0 * minimumx) {
        color = vec4(41, 128, 185, 255) / 255.0;
        color.a = 1.0;
    }
        
    // x axis
    if(fragPos3D.z > -1.0 * minimumz && fragPos3D.z < 1.0 * minimumz) {
        color = vec4(231, 76, 60, 255) / 255.0;
        color.a = 1.0;
    }

    return color;
}

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = data.fragProj * data.fragView * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

float computeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = data.fragProj * data.fragView * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * 0.01 * 1000) / (1000 + 0.01 - clip_space_depth * (1000 - 0.01)); // get linear value between 0.01 and 1000
    return linearDepth / 1000; // normalize
}

void main() {
    float cutoff = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + cutoff * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    float linearDepth = computeLinearDepth(fragPos3D) * 0.4;
    float fading = exp(-linearDepth * 70.0);
    float fading1 = exp(-linearDepth * 10.0);

    vec4 axisColor = drawAxis(fragPos3D);
    bool fragmentHasAxis = axisColor.a >= 1.0;

    outColor = (grid(fragPos3D, 1.0, .50, fragmentHasAxis));
    outColor += (grid(fragPos3D, 1.0 / 10.0, 0.96, fragmentHasAxis));
    outColor.a *= fading * 0.8;

    if(fragmentHasAxis) {
        outColor = axisColor;
        outColor.a = fading;
    }

    outColor *= float(cutoff > 0.0);

    // todo need to discard elements that are on the depth buffer so that we dont draw on top of meshes
}
