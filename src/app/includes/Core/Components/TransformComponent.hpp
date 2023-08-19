#pragma once
#include "gtc/quaternion.hpp"

struct TransformComponent {
    glm::vec3 m_Position = {};
    glm::quat m_Rotation = {};
    glm::vec3 m_Scale = glm::vec3(1.0f);
    glm::mat4 _matrix;
};
