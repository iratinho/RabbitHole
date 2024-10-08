#include "window.hpp"
#include "Components/InputComponent.hpp"
#include "Core/Scene.hpp"
#include "Core/InputSystem.hpp"

InputSystem::InputSystem(Window* window)
    : _window(window) {
    _window->GetKeyDelegate().AddRaw(this, &InputSystem::HandleInputKey);
    _window->GetMouseButtonDelegate().AddRaw(this, &InputSystem::HandleMouseButton);
}

bool InputSystem::Process(Scene* scene) {
    const auto view = scene->GetRegistry().view<InputComponent>();
    for (const auto entity : view) {
        auto& inputComponent = view.get<InputComponent>(entity);

        // Update mouse delta
        inputComponent.m_MouseDelta = _window->GetMouseDelta();
        inputComponent.m_WheelDelta = _window->GetMouseWheelDelta();

        // Update key pressed states for tracked keys in the input component
        for (auto& key : inputComponent.m_Keys) {
            if(_keyMap.contains(key.first)) {
                key.second = _keyMap[key.first].action;
            }
        }

        // Update mouse button states for tracked buttons in the input component
        for (auto& key : inputComponent.m_MouseButtons) {
            if(_mouseMap.contains(key.first)) {
                key.second = _mouseMap[key.first].action;
            }
        }
    }
    
    return true;
}

void InputSystem::HandleInputKey(int key, int scancode, int action, int modifier) {
    _keyMap[key] = {scancode, action, modifier};
}

void InputSystem::HandleMouseButton(int button, int action, int mods) {
    _mouseMap[button] = {action, mods};
}
