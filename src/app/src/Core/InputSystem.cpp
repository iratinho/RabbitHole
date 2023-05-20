#include "Core/InputSystem.hpp"

#include <window.hpp>
#include <GLFW/glfw3.h>

#include "Core/Components/InputComponent.hpp"
#include "Renderer/render_context.hpp"

bool InputSystem::Initialize(InitializationParams initializationParams) {
    m_Window = initializationParams.window_;

    if(m_Window == nullptr) {
        return false;
    }
    
    return true;
}

bool InputSystem::Process(entt::registry& registry) {
    auto view = registry.view<InputComponent>();

    for (const auto entity : view) {
        auto& inputComponent = view.get<InputComponent>(entity);

        // Update mouse delta
        inputComponent.m_MouseDelta = m_Window->GetMouseDelta();
        inputComponent.m_WheelDelta = m_Window->GetMouseWheelDelta().y;

        // Update key pressed states for tracked keys in the input component
        for (auto& key : inputComponent.m_Keys) {
            key.second = glfwGetKey(m_Window->GetWindow(), key.first) == GLFW_PRESS ? true : false;
        }

        // Update mouse button states for tracked buttons in the input component
        for (auto& key : inputComponent.m_MouseButtons) {
            key.second = glfwGetMouseButton(m_Window->GetWindow(), key.first) == GLFW_PRESS ? true : false;
        }
    }
    
    return true;
}
