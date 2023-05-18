#include "application.h"
#include "window.h"
#include "Renderer/simple_rendering.h"
#include "Renderer/RenderSystem.h"
#include "Core/CameraSystem.h"
#include "Core/GeometryLoaderSystem.h"
#include "Core/InputSystem.h"
#include "Core/Components/CameraComponent.h"
#include "Core/Components/InputComponent.h"
#include "Core/Components/SceneComponent.h"
#include "Core/Components/TransformComponent.h"
#include "Core/Components/UserInterfaceComponent.h"
#include "GLFW/glfw3.h"
#include "UI/UISystem.h"

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
        m_InputSystem = new InputSystem;
        m_CameraSystem = new CameraSystem;
        m_UISystem = new UISystem;
        _geometryLoaderSystem = new GeometryLoaderSystem;
        
        window::InitializationParams window_params {
            "Vulkan",
            800,
            600
        };

        window_params.resize_callback = &Application::HandleResize;
        window_params.drag_drop_callback = &Application::HandleDragAndDrop;
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

        if(!m_InputSystem->Initialize(renderer_params)) {
            return false;
        }

        if(!m_CameraSystem->Initialize(renderer_params)) {
            return false;
        }
        
        if(!render_system_->Initialize(renderer_params)) {
            std::cerr << "[Error]: Render system failed to initialize." << std::endl;

            return false;
        }

        if(!m_UISystem->Initialize(render_system_->GetRenderContext(), renderer_params)) {
            return false;
        }

        if(!_geometryLoaderSystem->Initialize(renderer_params)) {
            return false;
        }

        // Main view entity
        const auto entity = registry.create();
        
        // Temporary create a camera and transform component
        TransformComponent transformComponent {};
        transformComponent.m_Position = glm::vec3(-10.0f, 15.0f, -25.0f);
        // transformComponent.m_Position = glm::vec3(0.0f, 10.0f, 0.0f);

        CameraComponent cameraComponent {};
        cameraComponent.m_Fov = 120.0f;

        InputComponent inputComponent {};
        inputComponent.m_Keys.emplace(GLFW_KEY_W, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_S, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_D, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_A, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_E, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_Q, false);
        inputComponent.m_MouseButtons.emplace(GLFW_MOUSE_BUTTON_LEFT, false);

        // Data for this component will be populated in the UI System
        UserInterfaceComponent userInterfaceComponent {};

        SceneComponent sceneComponent {};
        
        registry.emplace<TransformComponent>(entity, transformComponent);
        registry.emplace<CameraComponent>(entity, cameraComponent);
        registry.emplace<InputComponent>(entity, inputComponent);
        registry.emplace<UserInterfaceComponent>(entity, userInterfaceComponent);
        registry.emplace<SceneComponent>(entity, sceneComponent);
        
        return true;
    }

    void Application::Shutdown() {
        glfwTerminate();
        delete main_window_;
    }

    void Application::Update() {
        while(main_window_ && !main_window_->ShouldWindowClose()) {
            main_window_->PoolEvents();
            m_UISystem->Process(registry);
            m_InputSystem->Process(registry);
            m_CameraSystem->Process(registry);
            render_system_->Process(registry);
            _geometryLoaderSystem->Process(registry);
            main_window_->ClearDeltas();
        }
    }

    void Application::HandleResize(const void* callback_context, int width, int height) {
        const auto* app = static_cast<const Application*>(callback_context);
        if(app && app->render_system_) {
            app->render_system_->HandleResize(width, height);
        }
    }

    void Application::HandleDragAndDrop(const void* callback_context, int count, const char** paths) {
        const auto* app = static_cast<const Application*>(callback_context);
        if(app && app->_geometryLoaderSystem) {
            std::cout << "[Info]: File Dragged!" << std::endl;
            for (int i = 0; i < count; ++i) {
                app->_geometryLoaderSystem->EnqueueFileLoad(paths[i]);
            }
        }
    }

}
