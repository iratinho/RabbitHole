#version 450

layout(binding=0) uniform sampler2D texSampler;

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

    // Ambient term
    float ambientStrength = 0.05;
    //vec3 ambientColor = vec3(1.0, 1.0, 1.0);
    vec3 ambientColor = diffuseSample;
    vec3 ambient = ambientColor * ambientStrength;

    // Diffuse term
    vec3 diffuseColor = diffuseSample;
    float diffuseCoeff = max(dot(surfaceNormal, lightVector), 0.0);
    vec3 diffuse = lightColor * diffuseColor * diffuseCoeff * lightIntensity;

    // Specular term
    float shininess = 100.0;
    vec3 halfVector = normalize(lightVector + cameraVector);
    float specularCoeff = pow(max(dot(surfaceNormal, halfVector), 0.0), shininess);
    //vec3 specularColor = vec3(0.0, 1.0, 0.0);
    vec3 specularColor = diffuseSample;
    vec3 specular = lightColor * specularColor * specularCoeff * lightIntensity;

    // Combine all components
    vec3 color = ambient + diffuse + specular;

    // Apply gamma correction to the final color
    //float gamma = 2.2;
   //color = pow(color, vec3(1.0 / gamma));

    fragColor = vec4(color, 1.0);
}