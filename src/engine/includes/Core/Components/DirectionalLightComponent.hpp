#pragma once
#include "glm/glm.hpp"

struct DirectionalLightComponent {
    glm::vec3 _color;
    glm::vec3 _direction;
    float _intensity; 
};
