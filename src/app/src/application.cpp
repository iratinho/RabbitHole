#include "application.h"
#include "window.h"
#include "Renderer/simple_rendering.h"
#include "Renderer/RenderSystem.h"
#include <iostream>

#include "Core/Components/CameraComponent.h"
#include "Core/Components/TransformComponent.h"
#include "GLFW/glfw3.h"

namespace app {
    Application::~Application() {
        Shutdown();
    }
    
    bool Application::Initialize() {
        if(!glfwInit()) {
            const int code = glfwGetError(nullptr);
            std::cerr << "[Error]: Failed to initialize glfw3 library. (Code: " <<  code << ")." << std::endl;
            return false;
        }

        main_window_ = new window::Window;
        render_system_ = new RenderSystem;

        window::InitializationParams window_params {
            "Vulkan",
            800,
            600
        };

        window_params.resize_callback = &Application::HandleResize;
        window_params.callback_context = this;
        
        if(!main_window_->Initialize(window_params)) {
            std::cerr << "[Error]: Failed to initialize the main window." << std::endl;
            return false;
        }

        const auto&[extensionCount, extensions] = main_window_->GetRequiredExtensions();
        
        const InitializationParams renderer_params {
            true,
            extensionCount,
            extensions,
            main_window_
        };
        
        if(!render_system_->Initialize(renderer_params)) {
            return false;
        }

        // Main view entity
        const auto entity = registry.create();
        
        // Temporary create a camera and transform component
        TransformComponent transformComponent {};
        transformComponent.m_Position = glm::vec3(-10.0f, 15.0f, -25.0f);

        CameraComponent cameraComponent {};
        cameraComponent.m_ViewMatrix = glm::lookAt(transformComponent.m_Position * -.5f, glm::vec3(0.0f), glm::vec3(0.0f, 1.f, 0.0f));
        cameraComponent.m_Fov = 65.0f;
        
        registry.emplace<TransformComponent>(entity, transformComponent);
        registry.emplace<CameraComponent>(entity, cameraComponent);

        return true;
    }

    void Application::Shutdown() {
        glfwTerminate();
        delete main_window_;
    }

    void Application::Update() {
        while(main_window_ && !main_window_->ShouldWindowClose()) {
            main_window_->PoolEvents();
            render_system_->Process(registry);
        }
    }

    void Application::HandleResize(const void* callback_context, int width, int height) {
        const auto* app = static_cast<const Application*>(callback_context);
        if(app && app->render_system_) {
            app->render_system_->HandleResize(width, height);
        }
    }
}
