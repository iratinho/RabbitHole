#pragma once
#include "glm/glm.hpp"
#include "Components/Common.hpp"

class DirectionalLightComponent : public CommonComponent {
public:
    DECLARE_CONSTRUCTOR(DirectionalLightComponent, CommonComponent)
    glm::vec3 _color;
    glm::vec3 _direction;
    float _intensity; 
};
