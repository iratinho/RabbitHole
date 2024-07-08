#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;
layout(location = 3) in mat4 fragView;
layout(location = 7) in mat4 fragProj;

vec4 grid(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

    if(drawAxis) {
        // z axis
        if(fragPos3D.x > -1.0 * minimumx && fragPos3D.x < 1.0 * minimumx)
            color.z = 1.0;
        // x axis
        if(fragPos3D.z > -1.0 * minimumz && fragPos3D.z < 1.0 * minimumz)
            color.x = 1.0;
    }

    return color;
}

vec4 smallGrid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    vec4 color = vec4(1.0, 1.0, 1.0, 1.0 - min(line, 1.0));
    return color;
}

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = fragProj * fragView * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}
float computeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = fragProj * fragView * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * 0.01 * 150) / (150 + 0.01 - clip_space_depth * (150 - 0.01)); // get linear value between 0.01 and 100
    return linearDepth / 150; // normalize
}
void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    float linearDepth = computeLinearDepth(fragPos3D) * 0.4;
    float fading = exp(-linearDepth * 450.0);

    outColor = ((grid(fragPos3D, 1.0, true) * 1.1) + grid(fragPos3D, 5.0, false) * 0.05) * float(t > 0); 
    outColor.a *= fading * 0.8;

    // if(outColor.a < 0.01) discard;

    // outColor.x = outColor.a;
    // outColor.y = outColor.a;
    // outColor.z = outColor.a;
 
    // gl_FragDepth = linearDepth;

    // gl_FragDepth = outColor.a * float(t > 0);
}
