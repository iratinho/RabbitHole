#include "Core/Camera.hpp"
#include "Core/Scene.hpp"
// TODO remove this from here
#include "GLFW/glfw3.h"

Camera::Camera(const Camera& camera) {
    this->_scene = camera._scene;
    this->_entity = camera._entity;
}

Camera::Camera(Camera&& camera) {
    this->_scene = camera._scene;
    this->_entity = camera._entity;

    camera._scene = nullptr;
}

Camera::Camera(Scene* scene, CameraInitializationParams params) {
    _scene = scene;
    _entity = scene->GetRegistry().create();

    CameraComponent cameraComponent {};
    cameraComponent.m_Fov = params._fov;

    TransformComponent transformComponent {};
    transformComponent.m_Position = params._position;

    InputComponent inputComponent {};
    inputComponent.m_Keys.emplace(GLFW_KEY_W, false);
    inputComponent.m_Keys.emplace(GLFW_KEY_S, false);
    inputComponent.m_Keys.emplace(GLFW_KEY_D, false);
    inputComponent.m_Keys.emplace(GLFW_KEY_A, false);
    inputComponent.m_Keys.emplace(GLFW_KEY_E, false);
    inputComponent.m_Keys.emplace(GLFW_KEY_Q, false);
    inputComponent.m_MouseButtons.emplace(GLFW_MOUSE_BUTTON_LEFT, false);

    scene->GetRegistry().emplace<TransformComponent>(_entity, transformComponent);
    scene->GetRegistry().emplace<CameraComponent>(_entity, cameraComponent);
    scene->GetRegistry().emplace<InputComponent>(_entity, inputComponent);

    scene->AddObject(std::move(*this));
}

const char* Camera::GetIdentifier() {
    return Camera::IDENTIFIER;
}

float Camera::GetFOV() {
    CameraComponent& component = _scene->GetComponents<CameraComponent>(_entity);
    return component.m_Fov;
}

glm::vec3 Camera::GetPosition() {
    TransformComponent& component = _scene->GetComponents<TransformComponent>(_entity);
    return component.m_Position;
}

glm::mat4 Camera::GetViewMatrix() {
    CameraComponent& component = _scene->GetComponents<CameraComponent>(_entity);
    return component.m_ViewMatrix;
}


