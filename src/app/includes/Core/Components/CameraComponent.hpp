#pragma once
#include <mat4x4.hpp>
#include <ext/matrix_transform.hpp>

struct CameraComponent {
    float m_Fov;
    glm::mat4 m_ViewMatrix = glm::mat4x4(1);
    glm::vec3 m_CameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
};
