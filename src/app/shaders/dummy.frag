#version 450

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 light_color;
layout(location = 1) in float light_intensity;
layout(location = 2) in vec3 light_position;
layout(location = 3) smooth in vec3 vertex_normal;
layout(location = 4) in vec3 fragPos;

void main()
{
    vec3 lightDirection = light_position - fragPos;
    vec3 objectColor = vec3(0.0, 0.0, 1.0);

    // Ambient term
    float ambientStrength = 0.01;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse term
    float lightCoeff = max(dot(normalize(vertex_normal), normalize(lightDirection)), 0.0);
    vec3 diffuseColor = light_color * lightCoeff * light_intensity;
    
    // Specular term 
    // vec3 halfVector = normalize(vertex_normal) + 


    fragColor = vec4((ambient + diffuseColor) * objectColor, 1.0);
    // fragColor = vec4(normalize(vertex_normal), 1.0);
}