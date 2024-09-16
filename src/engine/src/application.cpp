#include "application.hpp"
#include "window.hpp"
#include "Renderer/RenderSystemV2.hpp"
#include "Core/CameraSystem.hpp"
#include "Core/GeometryLoaderSystem.hpp"
#include "Core/InputSystem.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/InputComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/UserInterfaceComponent.hpp"
#include "Components/DirectionalLightComponent.hpp"
#include "Components/PrimitiveComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "GLFW/glfw3.h"
#include "UI/UISystem.hpp"
//#include <grpc/grpc.h>

#include "Core/Scene.hpp"
#include "Core/Camera.hpp"
#include "Core/Light.hpp"
#include "Core/MeshObject.hpp"

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
                        
        _mainWindow = new Window;
        _renderSystem = new RenderSystemV2;
        _inputSystem = new InputSystem;
        _cameraSystem = new CameraSystem;
//        _uiSystem = new UISystem;
        _geometryLoaderSystem = new GeometryLoaderSystem;
        _scene = new Scene;
        
        WindowInitializationParams windowParams {
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

//        if(!_uiSystem->Initialize(_renderSystem->GetRenderContext(), renderer_params)) {
//            return false;
//        }

        if(!_geometryLoaderSystem->Initialize(renderer_params)) {
            return false;
        }
        
        CreateDefaultCamera();
        CreateDefaultLights();
        CreateFloorGridMesh();
        
        // Enable matcapmode
//        _scene->ToggleMatCapMode();
        
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
        entt::entity entity = _scene->GetRegistry().create();
        
        CameraComponent& cameraComponent = _scene->GetRegistry().emplace<CameraComponent>(entity);
        cameraComponent.m_Fov = 120.0f;
        cameraComponent._isActive = true;
        
        TransformComponent& transformComponent = _scene->GetRegistry().emplace<TransformComponent>(entity);
        transformComponent.m_Position = glm::vec3(-10.0f, 15.0f, -25.0f);

        InputComponent& inputComponent = _scene->GetRegistry().emplace<InputComponent>(entity);
        inputComponent.m_Keys.emplace(GLFW_KEY_W, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_S, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_D, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_A, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_E, false);
        inputComponent.m_Keys.emplace(GLFW_KEY_Q, false);
        inputComponent.m_MouseButtons.emplace(GLFW_MOUSE_BUTTON_LEFT, false);
    }
    
    void Application::CreateDefaultLights() {
        entt::entity entity = _scene->GetRegistry().create();
        
        DirectionalLightComponent& directionalLightComponent = _scene->GetRegistry().emplace<DirectionalLightComponent>(entity);
        directionalLightComponent._color = glm::vec3(1.0f, 1.0f, 1.0f);
        directionalLightComponent._direction = glm::vec3(0.0f, 1.0f, 0.f);
        directionalLightComponent._intensity = 10.0f;
    }
    
    void Application::CreateFloorGridMesh() {
        const std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2};

        const std::vector<VertexData> vertexData = {
            {{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, // 0
            {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // 1
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // 2
            {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        };
        
        entt::entity primitiveEntity = _scene->GetRegistry().create();
        
        _scene->GetRegistry().emplace<TransformComponent>(primitiveEntity);
        
        PrimitiveProxyComponentCPU& proxy = _scene->GetRegistry().emplace<PrimitiveProxyComponentCPU>(primitiveEntity);
        proxy._indices = std::move(indices);
        proxy._vertexData = std::move(vertexData);
        
        GridMaterialComponent& gridMaterialComponent = _scene->GetRegistry().emplace<GridMaterialComponent>(primitiveEntity);
        gridMaterialComponent._identifier = "floorGridMaterial";
                
        entt::entity meshEntity = _scene->GetRegistry().create();
        
        MeshComponentNew& meshComponent = _scene->GetRegistry().emplace<MeshComponentNew>(meshEntity);
        meshComponent._identifier = "FloorGrid";
        meshComponent._primitives.push_back(primitiveEntity);
        
        _scene->GetRegistry().emplace<TransformComponent>(meshEntity);
    }
    
    void Application::HandleResize(const void* callback_context, int width, int height) {
        const auto* app = static_cast<const Application*>(callback_context);
        if(app && app->_renderSystem) {
//            app->_renderSystem->HandleResize(width, height);
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
