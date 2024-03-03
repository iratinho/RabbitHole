#pragma once
#include "Components/Common.hpp"
#include "glm/gtc/quaternion.hpp"

class TransformComponent : public CommonComponent {    
public:
    DECLARE_CONSTRUCTOR(TransformComponent, CommonComponent)

    glm::vec3 m_Position = {};
    glm::quat m_Rotation = {};
    glm::vec3 m_Scale = glm::vec3(1.0f);
    glm::mat4 _matrix;
};
