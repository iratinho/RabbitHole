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

bool CameraSystem::Process(Scene* scene) {
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

void CameraSystem::ComputeArcBallCamera(Scene* scene)
{
    auto view = scene->GetRegistry().view<TransformComponent, CameraComponent, InputComponent>();
    for (const auto entity : view) {
        auto [transformComponent, cameraComponent, inputComponent] = view.get<TransformComponent, CameraComponent, InputComponent>(entity);
        
        if(!cameraComponent._isActive) {
            continue;
        }
        
        const bool bIsRightMousePressed = inputComponent.m_MouseButtons.contains(GLFW_MOUSE_BUTTON_LEFT) && inputComponent.m_MouseButtons[GLFW_MOUSE_BUTTON_LEFT];

        if(bIsRightMousePressed) {
            m_Window->HideCursor();
        }
        else {
            m_Window->ShowCursor();
        }

        constexpr glm::vec3 pivot = glm::vec3();
        constexpr glm::vec3 upVector = glm::vec3(0.0f, -1.0f,  0.0f);
        glm::vec3 finalLocation = glm::vec3();
            
        // Calculate a vector that represents the camera location to the pivot point
        glm::vec3 zoomVector = glm::normalize(transformComponent.m_Position - pivot) * inputComponent.m_WheelDelta * -2.0f;
        transformComponent.m_Position+=zoomVector;

        if(bIsRightMousePressed && inputComponent.m_MouseDelta != glm::vec2(0.0f, 0.0f))
        {
            glm::vec3 rightVector = glm::transpose(cameraComponent.m_ViewMatrix)[0];
            constexpr float mouseSensitivity = 2.0f;
            glm::vec2 mouseDelta = inputComponent.m_MouseDelta;
            mouseDelta *= mouseSensitivity;
                
            // In the x axis we have a full rotation
            float deltaAngleX = -1.0f * mouseDelta.x * (2 * glm::pi<float>() / (float)m_Window->GetWindowSurfaceSize().x);
                
            // In the Y axis we only have half rotation
            float deltaAngleY = mouseDelta.y * (glm::pi<float>() / (float)m_Window->GetWindowSurfaceSize().y);
                
            glm::mat4x4 rotationMatrix(1.0f);
                
            // Lets rotate around the Up vector
            rotationMatrix = glm::rotate(rotationMatrix, deltaAngleX, upVector);

            // Lets rotate around the
            rotationMatrix = glm::rotate(rotationMatrix, deltaAngleY, rightVector);

            // Lets calculate the final position for the camera
            finalLocation = (transformComponent.m_Position - pivot) + pivot;
            transformComponent.m_Position = rotationMatrix * glm::vec4(finalLocation.x, finalLocation.y, finalLocation.z, 1.0f);
        }
        else
        {
            finalLocation = transformComponent.m_Position;
        }

        // Calculate view matrix for this camera
        cameraComponent.m_ViewMatrix = glm::lookAt(finalLocation, pivot, upVector);
    }
}
