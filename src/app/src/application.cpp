#include "application.hpp"
#include "window.hpp"
#include "Renderer/simple_rendering.hpp"
#include "Renderer/RenderSystem.hpp"
#include "Core/CameraSystem.hpp"
#include "Core/GeometryLoaderSystem.hpp"
#include "Core/InputSystem.hpp"
#include "Core/Components/CameraComponent.hpp"
#include "Core/Components/InputComponent.hpp"
#include "Core/Components/MeshComponent.hpp"
#include "Core/Components/TransformComponent.hpp"
#include "Core/Components/UserInterfaceComponent.hpp"
#include "Core/Components/DirectionalLightComponent.hpp"
#include "GLFW/glfw3.h"
#include "UI/UISystem.hpp"
#include <grpc/grpc.h>

#include "Core/Scene.hpp"
#include "Core/Camera.hpp"
#include "Core/Light.hpp"

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
        _scene = new Scene;
        
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
        
        CreateDefaultCamera();
        CreateDefaultLights();
                
        return true;
    }
    
    void Application::Shutdown() {
        glfwTerminate();
        delete _mainWindow;
    }

    void Application::Update() {
        while(_mainWindow && !_mainWindow->ShouldWindowClose()) {
            _mainWindow->PoolEvents();
//            _uiSystem->Process();
            _inputSystem->Process(_scene);
            _cameraSystem->Process(_scene);
            _renderSystem->Process(_scene);
            _geometryLoaderSystem->Process(_scene);
            _mainWindow->ClearDeltas();
        }
    }
    
    void Application::CreateDefaultCamera() {
        CameraInitializationParams params;
        params._position = glm::vec3(-10.0f, 15.0f, -25.0f);
        params._fov = 120.0f;
        const Camera& camera = CameraFactory::MakeObject(_scene, params);
        _scene->SetActiveCamera(camera);
    }
    
    void Application::CreateDefaultLights() {
        DirectionalLightComponent directionalLightComponent;
        directionalLightComponent._color = glm::vec3(1.0f, 1.0f, 1.0f);
        directionalLightComponent._direction = glm::vec3(0.0, -10.0f, 0.0f);
        directionalLightComponent._intensity = 1.0f;
        
        LightInitializationParams params;
        params.lightComponent = std::move(directionalLightComponent);
        const Light& light = LightFactory::MakeObject(_scene, params);
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
