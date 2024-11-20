#include "Core/Scene.hpp"
#include "Core/CameraSystem.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/InputComponent.hpp"
#include "Components/TransformComponent.hpp"
#include <window.hpp>
#include <GLFW/glfw3.h>

bool CameraSystem::Initialize(Window* window) {
    m_Window = window;

    if(!m_Window)
        return false;

    return true;
}

bool CameraSystem::Process(Scene* scene) const {
    ComputeArcBallCamera(scene);
    // ComputeFirstPersonCamera(registry);
    return true;
}

//void CameraSystem::ComputeFirstPersonCamera(entt::registry& registry)
//{
//    auto view = registry.view<TransformComponent, CameraComponent, InputComponent>();
//
//    for (const auto entity : view) {
//        auto [transformComponent, cameraComponent, inputComponent] = view.get<TransformComponent, CameraComponent, InputComponent>(entity);
//
//        constexpr float movementMultiplier = 0.01f;
//        constexpr glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f,  0.0f);
//        
//        glm::vec3& cameraFront = cameraComponent.m_CameraFront;
//
//        bool bIsRightMousePressed = inputComponent.m_MouseButtons.contains(GLFW_MOUSE_BUTTON_LEFT) && inputComponent.m_MouseButtons[GLFW_MOUSE_BUTTON_LEFT];
//
//        if(bIsRightMousePressed) {
//            m_Window->HideCursor();
//        }
//        else {
//            m_Window->ShowCursor();
//        }
//        
//        // Mouse movement
//        if(bIsRightMousePressed && inputComponent.m_MouseDelta != glm::vec2(0.0f, 0.0f)){
//
//            // Apply mouse sensitivity
//            constexpr float mouseSensitivity = 2.0f;
//            glm::vec2 mouseDelta = inputComponent.m_MouseDelta;
//            mouseDelta *= mouseSensitivity;
//            /*
//             * Note about rotations:
//             * Pitch = mouseDelta.y
//             * Yaw = mouseDelta.x
//             */
//            
//            // Enforce rotation limitations
//            if(mouseDelta.y > 89.0f)
//                mouseDelta.y = 89.0f;
//            if(mouseDelta.y < -89.0f)
//                mouseDelta.y = -89.0f;
//
//            // Update camera rotation
//            transformComponent.m_Rotation.x -= glm::radians(mouseDelta.x);
//            transformComponent.m_Rotation.y -= glm::radians(mouseDelta.y) ;
//            transformComponent.m_Rotation.z = 0.0f;
//            
//            // With this rotations values calculate proper direction of movement
//            glm::vec3 directionVector;
//            directionVector.x = cos(glm::radians(transformComponent.m_Rotation.x)) * cos(glm::radians(transformComponent.m_Rotation.y));
//            directionVector.y = sin(glm::radians(transformComponent.m_Rotation.y));
//            directionVector.z = sin(glm::radians(transformComponent.m_Rotation.x)) * cos(glm::radians(transformComponent.m_Rotation.y));
//            cameraFront = glm::normalize(directionVector);
//        }
//
//        // Movement
//        {
//            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
//            glm::vec3 cameraUpDirection = glm::normalize(glm::cross(cameraRight, cameraFront));
//        
//            // Forward
//            if (inputComponent.m_Keys.contains(GLFW_KEY_W) && inputComponent.m_Keys[GLFW_KEY_W]) {
//                transformComponent.m_Position += movementMultiplier * cameraFront;
//            }
//
//            // Backwards
//            if (inputComponent.m_Keys.contains(GLFW_KEY_S) && inputComponent.m_Keys[GLFW_KEY_S]) {
//                transformComponent.m_Position -= movementMultiplier * cameraFront;
//            }
//
//            // Right
//            if (inputComponent.m_Keys.contains(GLFW_KEY_D) && inputComponent.m_Keys[GLFW_KEY_D]) {
//                transformComponent.m_Position += movementMultiplier * cameraRight;
//            }
//
//            // Left
//            if (inputComponent.m_Keys.contains(GLFW_KEY_A) && inputComponent.m_Keys[GLFW_KEY_A]) {
//                transformComponent.m_Position -= movementMultiplier * cameraRight;
//            }
//
//            // Up
//            if (inputComponent.m_Keys.contains(GLFW_KEY_E) && inputComponent.m_Keys[GLFW_KEY_E]) {
//                transformComponent.m_Position -= movementMultiplier * cameraUpDirection;
//            }
//
//            // Down
//            if (inputComponent.m_Keys.contains(GLFW_KEY_Q) && inputComponent.m_Keys[GLFW_KEY_Q]) {
//                transformComponent.m_Position += movementMultiplier * cameraUpDirection;
//            }
//        }
//        
//        // Calculate view matrix for this camera
//        cameraComponent.m_ViewMatrix = glm::lookAt(transformComponent.m_Position, transformComponent.m_Position + cameraFront, cameraUp);
//    }
//}

void CameraSystem::ComputeArcBallCamera(Scene* scene) const {
    const auto view = scene->GetRegistry().view<TransformComponent, CameraComponent, InputComponent>();
    for (const auto entity : view)
    {
        auto [transformComponent, cameraComponent, inputComponent] = view.get<TransformComponent, CameraComponent, InputComponent>(entity);
        
        if(!cameraComponent._isActive) {
            continue;
        }

        const bool bIsRightMousePressed = inputComponent.m_MouseButtons.contains(GLFW_MOUSE_BUTTON_RIGHT) && inputComponent.m_MouseButtons[GLFW_MOUSE_BUTTON_RIGHT];
        const bool bIsCtrlPressed = inputComponent.m_Keys.contains(GLFW_KEY_LEFT_CONTROL) && inputComponent.m_Keys[GLFW_KEY_LEFT_CONTROL];
        const bool bIsShiftPressed = inputComponent.m_Keys.contains(GLFW_KEY_LEFT_SHIFT) && inputComponent.m_Keys[GLFW_KEY_LEFT_SHIFT];

        if(bIsRightMousePressed) {
            m_Window->HideCursor();
        }
        else {
            m_Window->ShowCursor();
        }

        glm::vec3 pivot = cameraComponent._currentPivotPosition;
        glm::vec3 cameraPosition = transformComponent.m_Position;
        glm::vec3 direction = glm::normalize(cameraPosition - pivot);
        glm::vec2 mouseDelta = inputComponent.m_MouseDelta;
        float radius = cameraComponent._radius;
        float rotationX = 0.0f;
        float rotationY = 0.0f;

        // Update pivot location
        if(bIsShiftPressed) {
            glm::vec2 mouseWheelDelta = inputComponent.m_WheelDelta;
            constexpr float panSensitivity = 0.2f;
            glm::vec3 right = glm::transpose(cameraComponent.m_ViewMatrix)[0];
            auto up = glm::transpose(cameraComponent.m_ViewMatrix)[1];

            pivot += glm::vec3(up) * panSensitivity * mouseWheelDelta.y;
            pivot += glm::vec3(right) * panSensitivity * mouseWheelDelta.x;

            cameraComponent._currentPivotPosition = pivot;
        }

        // Update rotation input values
        if(!bIsCtrlPressed && bIsRightMousePressed && inputComponent.m_MouseDelta != glm::vec2(0.0f, 0.0f)) {
            float rotationRate = 0.5f;

            rotationX = rotationRate * (-1.0f * mouseDelta.x * (2 * glm::pi<float>() / (float)m_Window->GetWindowSurfaceSize().x));
            rotationY = rotationRate * (mouseDelta.y * (glm::pi<float>() / (float)m_Window->GetWindowSurfaceSize().y));
        }

        // Update radius
        if(bIsCtrlPressed) {
            constexpr float zoomFactor = 0.2f;
            radius = radius + (-inputComponent.m_WheelDelta.y * zoomFactor);
        }

        direction *= radius;

        glm::vec3 upVector = glm::transpose(cameraComponent.m_ViewMatrix)[1];
        glm::vec3 rightVector = glm::transpose(cameraComponent.m_ViewMatrix)[0];

        glm::mat4x4 rotationMatrix(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, rotationX, upVector);
        rotationMatrix = glm::rotate(rotationMatrix, rotationY, rightVector);

        direction = rotationMatrix * glm::vec4(direction, 0.0f);

        glm::vec3 finalPosition = pivot + direction;
        cameraComponent._radius = radius;
        transformComponent.m_Position = finalPosition;
        
#ifdef VULKAN_BACKEND
        cameraComponent.m_ViewMatrix = glm::lookAt(finalPosition, pivot, glm::vec3(0.0f, -1.0f, 0.0f));
#else
        // In webgpu inverts the y axis and the scale so it matches the coordinate system
        cameraComponent.m_ViewMatrix = glm::lookAt(finalPosition, pivot, glm::vec3(0.0f, 1.0f, 0.0f));
#endif
        
    }
}
