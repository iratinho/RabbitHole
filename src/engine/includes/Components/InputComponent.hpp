#pragma once
#include "Components/Common.hpp"

class InputComponent : public CommonComponent {
public:
    DECLARE_CONSTRUCTOR(InputComponent, CommonComponent)
    std::unordered_map<int, bool> m_Keys;
    std::unordered_map<int, bool> m_MouseButtons;
    glm::vec2 m_MouseDelta = glm::vec2(0.0f, 0.0f);
    float m_WheelDelta = 0.0f;
};
