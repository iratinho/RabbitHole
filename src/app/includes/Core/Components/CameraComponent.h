#pragma once
#include <mat4x4.hpp>

struct CameraComponent {
    float m_Fov;
    glm::mat4 m_ViewMatrix;
};
