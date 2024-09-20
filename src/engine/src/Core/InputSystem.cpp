#include "Core/InputSystem.hpp"

#include <window.hpp>
#include <GLFW/glfw3.h>

#include "Components/InputComponent.hpp"
#include "Core/Scene.hpp"

InputSystem::InputSystem(Window* window)
    : _window(window) {
}

bool InputSystem::Process(Scene* scene) const {
    const auto view = scene->GetRegistry().view<InputComponent>();
    for (const auto entity : view) {
        auto& inputComponent = view.get<InputComponent>(entity);

        // Update mouse delta
        inputComponent.m_MouseDelta = _window->GetMouseDelta();
        inputComponent.m_WheelDelta = _window->GetMouseWheelDelta().y;

        // Update key pressed states for tracked keys in the input component
        for (auto& key : inputComponent.m_Keys) {
            key.second = glfwGetKey(_window->GetWindow(), key.first) == GLFW_PRESS ? true : false;
        }

        // Update mouse button states for tracked buttons in the input component
        for (auto& key : inputComponent.m_MouseButtons) {
            key.second = glfwGetMouseButton(_window->GetWindow(), key.first) == GLFW_PRESS ? true : false;
        }
    }
    
    return true;
}
