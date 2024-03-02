#version 450

// Push Constants
layout(push_constant) uniform PushConstants {
    vec3 eyePosition;
} pushConstant;

// Inputs
layout(location = 0) in vec3 fragNormal;

// Outputs
layout(location = 0) out vec4 fragColor;

// Samplers
layout(set=0, binding=0) uniform sampler2D texSampler;

void main() {
    vec3 r = reflect(pushConstant.eyePosition, fragNormal);
    float m = 2.0 * sqrt( pow( r.x, 2.0) + pow( r.y, 2.0 ) + pow( r.z + 1.0, 2.0 ) );
    vec2 vN = r.xy / m + 0.5;

    fragColor = texture(texSampler, vN);
}