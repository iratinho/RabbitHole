#pragma once
#include "Components/Common.hpp"
#include "glm/gtc/quaternion.hpp"
#include "entt/fwd.hpp"

class TransformComponent : public CommonComponent {    
public:
    DECLARE_CONSTRUCTOR(TransformComponent, CommonComponent)

    // Later we can have utilities that can build the computed matrix directly when modify data in this component
    glm::vec3 m_Position = {};
    glm::quat m_Rotation = {};
    glm::vec3 m_Scale = glm::vec3(1.0f);
    bool _isRootTransform = false;
    
    glm::mat4 _matrix = glm::identity<glm::mat4>(); // Imported matrix
    std::optional<glm::mat4> _computedMatrix;
    std::vector<uint32_t> _childs;
};
