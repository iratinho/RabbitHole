#include "application.hpp"
#include "window.hpp"
#include "Renderer/simple_rendering.hpp"
#include "Renderer/RenderSystem.hpp"
#include "Core/CameraSystem.hpp"
#include "Core/GeometryLoaderSystem.hpp"
#include "Core/InputSystem.hpp"
#include "Core/Components/CameraComponent.hpp"
#include "Core/Components/InputComponent.hpp"
#include "Core/Components/SceneComponent.hpp"
#include "Core/Components/TransformComponent.hpp"
#include "Core/Components/UserInterfaceComponent.hpp"
#include "GLFW/glfw3.h"
#include "UI/UISystem.hpp"

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

        _mainWindow = new window::Window;
        _renderSystem = new RenderSystem;
        _inputSystem = new InputSystem;
        _cameraSystem = new CameraSystem;
        _uiSystem = new UISystem;
        _geometryLoaderSystem = new GeometryLoaderSystem;
        
        window::InitializationParams windowParams {
            "Vulkan",
            800,
            600
        };

        windowParams.resize_callback = &Application::HandleResize;
        windowParams.drag_drop_callback = &Application::HandleDragAndDrop;
        windowParams.callback_context = this;
        
        if(!_mainWindow->Initialize(windowParams)) {
            std::cerr << "[Error]: Failed to initialize the main window." << std::endl;
            return false;
        }

        const auto&[extensionCount, extensions] = _mainWindow->GetRequiredExtensions();
        
        const InitializationParams renderer_params {
                true,
                extensionCount,
                extensions,
                _mainWindow
        };

        if(!_inputSystem->Initialize(renderer_params)) {
            return false;
        }

        if(!_cameraSystem->Initialize(renderer_params)) {
            return false;
        }
        
        if(!_renderSystem->Initialize(renderer_params)) {
            std::cerr << "[Error]: Render system failed to initialize." << std::endl;

            return false;
        }

        if(!_uiSystem->Initialize(_renderSystem->GetRenderContext(), renderer_params)) {
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
        delete _mainWindow;
    }

    void Application::Update() {
        while(_mainWindow && !_mainWindow->ShouldWindowClose()) {
            _mainWindow->PoolEvents();
            _uiSystem->Process(registry);
            _inputSystem->Process(registry);
            _cameraSystem->Process(registry);
            _renderSystem->Process(registry);
            _geometryLoaderSystem->Process(registry);
            _mainWindow->ClearDeltas();
        }
    }

    void Application::HandleResize(const void* callback_context, int width, int height) {
        const auto* app = static_cast<const Application*>(callback_context);
        if(app && app->_renderSystem) {
            app->_renderSystem->HandleResize(width, height);
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
