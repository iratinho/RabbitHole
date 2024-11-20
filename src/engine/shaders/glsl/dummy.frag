#version 450

layout(set=2, binding=0) uniform sampler2D texSampler;

layout(location = 0) in vec3 lightColor;
layout(location = 1) in float lightIntensity;
layout(location = 2) in vec3 lightDirection;
layout(location = 3) in vec3 vertexNormal;
layout(location = 4) in vec3 fragPosition;
layout(location = 5) in vec3 cameraPosition;
layout(location = 6) in vec2 tCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    vec3 lightVector = normalize(lightDirection);
    vec3 cameraVector = normalize(cameraPosition - fragPosition);
    vec3 surfaceNormal = normalize(vertexNormal);
    vec3 diffuseSample = texture(texSampler, tCoords).rgb;

    float ambientStrength = 0.05;
    vec3 ambientColor = diffuseSample;
    vec3 ambient = ambientColor * ambientStrength;

    // Diffuse term
    vec3 diffuseColor = diffuseSample;
    float diffuseCoeff = max(dot(surfaceNormal, lightVector), 0.0);
    vec3 diffuse =  lightColor * lightIntensity * diffuseColor * diffuseCoeff;

    // Specular term
    float shininess = 100.0;
    vec3 halfVector = normalize(lightVector + cameraVector);
    float specularCoeff = pow(max(dot(surfaceNormal, halfVector), 0.0), shininess);
    vec3 specularColor = diffuseSample; // Dont have input for specular color, use the surface color to reflect light
    vec3 specular = lightColor * lightIntensity * specularColor * specularCoeff;

    // Combine all components
    vec3 color = ambient + diffuse + specular;

    fragColor = vec4(color, 1.0);
}