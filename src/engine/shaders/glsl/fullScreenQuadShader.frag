#version 450

layout(binding=0) uniform sampler2D texSampler;
layout(location = 0) in vec2 texCoords;
layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 texColor = texture(texSampler, texCoords);
    fragColor = texColor;
    // fragColor = vec4(texColor.a, texColor.a, texColor.a, 1.0);
}