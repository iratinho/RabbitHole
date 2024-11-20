#version 450

layout(set=1, binding=0) uniform sampler2D texSampler;

// Inputs
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 e;

// Outputs
layout(location = 0) out vec4 fragColor;

void main() {
    vec3 r = reflect( e, fragNormal );
    float m = 2. * sqrt( pow( r.x, 2. ) + pow( r.y, 2. ) + pow( r.z + 1., 2. ) );
    vec2 vN = r.xy / m + .5;

    vec3 base = texture( texSampler, vN ).rgb;

    fragColor = vec4( base, 1. );
}