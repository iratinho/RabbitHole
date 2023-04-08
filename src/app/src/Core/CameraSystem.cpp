#include "Core/CameraSystem.h"

#include <window.h>
#include <GLFW/glfw3.h>

#include "Core/Components/CameraComponent.h"
#include "Core/Components/InputComponent.h"
#include "Core/Components/TransformComponent.h"
#include "Renderer/render_context.h"

bool CameraSystem::Initialize(InitializationParams initialization_params) {
    m_Window = initialization_params.window_;

    if(!m_Window)
        return false;
    
    return true;
}

bool CameraSystem::Process(entt::registry& registry) {
    // TODO want to also create a orbit camera
    ComputeFirstPersonCamera(registry);
    return true;
}

void CameraSystem::ComputeFirstPersonCamera(entt::registry& registry)
{
    auto view = registry.view<TransformComponent, CameraComponent, InputComponent>();

    for (const auto entity : view) {
        auto [transformComponent, cameraComponent, inputComponent] = view.get<TransformComponent, CameraComponent, InputComponent>(entity);

        constexpr float movementMultiplier = 0.01f;
        constexpr glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f,  0.0f);
        
        glm::vec3& cameraFront = cameraComponent.m_CameraFront;

        bool bIsRightMousePressed = inputComponent.m_MouseButtons.contains(GLFW_MOUSE_BUTTON_LEFT) && inputComponent.m_MouseButtons[GLFW_MOUSE_BUTTON_LEFT];

        if(bIsRightMousePressed) {
            m_Window->HideCursor();
        }
        else {
            m_Window->ShowCursor();
        }
        
        // Mouse movement
        if(bIsRightMousePressed && inputComponent.m_MouseDelta != glm::vec2(0.0f, 0.0f)){

            // Apply mouse sensitivity
            constexpr float mouseSensitivity = 2.0f;
            glm::vec2 mouseDelta = inputComponent.m_MouseDelta;
            mouseDelta *= mouseSensitivity;
            /*
             * Note about rotations:
             * Pitch = mouseDelta.y
             * Yaw = mouseDelta.x
             */
            
            // Enforce rotation limitations
            if(mouseDelta.y > 89.0f)
                mouseDelta.y = 89.0f;
            if(mouseDelta.y < -89.0f)
                mouseDelta.y = -89.0f;

            // Update camera rotation
            transformComponent.m_Rotation.x -= glm::radians(mouseDelta.x);
            transformComponent.m_Rotation.y -= glm::radians(mouseDelta.y) ;
            transformComponent.m_Rotation.z = 0.0f;
            
            // With this rotations values calculate proper direction of movement
            glm::vec3 directionVector;
            directionVector.x = cos(glm::radians(transformComponent.m_Rotation.x)) * cos(glm::radians(transformComponent.m_Rotation.y));
            directionVector.y = sin(glm::radians(transformComponent.m_Rotation.y));
            directionVector.z = sin(glm::radians(transformComponent.m_Rotation.x)) * cos(glm::radians(transformComponent.m_Rotation.y));
            cameraFront = glm::normalize(directionVector);
        }

        // Movement
        {
            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
            glm::vec3 cameraUpDirection = glm::normalize(glm::cross(cameraRight, cameraFront));
        
            // Forward
            if (inputComponent.m_Keys.contains(GLFW_KEY_W) && inputComponent.m_Keys[GLFW_KEY_W]) {
                transformComponent.m_Position += movementMultiplier * cameraFront;
            }

            // Backwards
            if (inputComponent.m_Keys.contains(GLFW_KEY_S) && inputComponent.m_Keys[GLFW_KEY_S]) {
                transformComponent.m_Position -= movementMultiplier * cameraFront;
            }

            // Right
            if (inputComponent.m_Keys.contains(GLFW_KEY_D) && inputComponent.m_Keys[GLFW_KEY_D]) {
                transformComponent.m_Position += movementMultiplier * cameraRight;
            }

            // Left
            if (inputComponent.m_Keys.contains(GLFW_KEY_A) && inputComponent.m_Keys[GLFW_KEY_A]) {
                transformComponent.m_Position -= movementMultiplier * cameraRight;
            }

            // Up
            if (inputComponent.m_Keys.contains(GLFW_KEY_E) && inputComponent.m_Keys[GLFW_KEY_E]) {
                transformComponent.m_Position -= movementMultiplier * cameraUpDirection;
            }

            // Down
            if (inputComponent.m_Keys.contains(GLFW_KEY_Q) && inputComponent.m_Keys[GLFW_KEY_Q]) {
                transformComponent.m_Position += movementMultiplier * cameraUpDirection;
            }
        }
        
        // Calculate view matrix for this camera
        cameraComponent.m_ViewMatrix = glm::lookAt(transformComponent.m_Position, transformComponent.m_Position + cameraFront, cameraUp);
        cameraComponent.m_Fov = 120.0f;
    }
}
