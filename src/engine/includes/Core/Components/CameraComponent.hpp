#pragma once
#include "glm/mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp"

struct CameraComponent {
    float m_Fov;
    glm::mat4 m_ViewMatrix = glm::mat4x4(1);
    glm::vec3 m_CameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
};
