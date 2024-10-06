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
#include "Components/DirectionalLightComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "GLFW/glfw3.h"
//#include <grpc/grpc.h>

#include "Core/Scene.hpp"
#include "Core/Camera.hpp"

namespace app {
    Application::~Application() {
        Shutdown();
    }
    
    bool Application::Initialize() {
        try {
            InitializeInternal();
        } catch (const std::exception& e) {
            std::cerr << "[Error]: " << e.what() << std::endl;
            Shutdown();
            return false;
        }
        
        CreateDefaultCamera();
        CreateDefaultLights();
        CreateFloorGridMesh();

        return true;
    }
    
    void Application::Shutdown() const {
        glfwTerminate();
        delete _mainWindow;
    }

    void Application::Update() const {
        while(_mainWindow && !_mainWindow->ShouldWindowClose()) {
            _mainWindow->PoolEvents();

            if(InputSystem* inputSystem = _mainWindow->GetInputSystem()) {
                inputSystem->Process(_scene);
            }

            _cameraSystem->Process(_scene);
            _renderSystem->Process(_scene);
            _geometryLoaderSystem->Process(_scene);
            _mainWindow->ClearDeltas();
        }
    }

    void Application::InitializeInternal() {
        if(!glfwInit()) {
            const int code = glfwGetError(nullptr);
            throw std::runtime_error("[Error]: Failed to initialize glfw3 library. (Code: " + std::to_string(code) + ").");
        }

        _mainWindow = new Window;
        _renderSystem = new RenderSystemV2;
        _cameraSystem = new CameraSystem;
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
            throw std::runtime_error("[Error]: Failed to initialize the main window.");
        }

        if(!_cameraSystem->Initialize(_mainWindow)) {
            throw std::runtime_error("[Error]: Camera system failed to initialize.");
        }

        if(!_renderSystem->Initialize(_mainWindow)) {
            throw std::runtime_error("[Error]: Render system failed to initialize.");
        }

        if(!_geometryLoaderSystem->Initialize()) {
            throw std::runtime_error("[Error]: Geometry loader system failed to initialize.");
        }
    }

    void Application::CreateDefaultCamera() const {
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
    
    void Application::CreateDefaultLights() const {
        const entt::entity entity = _scene->GetRegistry().create();
        
        DirectionalLightComponent& directionalLightComponent = _scene->GetRegistry().emplace<DirectionalLightComponent>(entity);
        directionalLightComponent._color = glm::vec3(1.0f, 1.0f, 1.0f);
        directionalLightComponent._direction = glm::vec3(0.0f, 1.0f, 0.f);
        directionalLightComponent._intensity = 1.0f;
    }
    
    void Application::CreateFloorGridMesh() const {
        const std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2};

        const std::vector<VertexData> vertexData = {
            {{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, // 0
            {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // 1
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // 2
            {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        };
        
        const entt::entity primitiveEntity = _scene->GetRegistry().create();
        
        _scene->GetRegistry().emplace<TransformComponent>(primitiveEntity);
        
        PrimitiveProxyComponentCPU& proxy = _scene->GetRegistry().emplace<PrimitiveProxyComponentCPU>(primitiveEntity);
        proxy._indices = indices;
        proxy._vertexData = vertexData;
        
        GridMaterialComponent& gridMaterialComponent = _scene->GetRegistry().emplace<GridMaterialComponent>(primitiveEntity);
        gridMaterialComponent._identifier = "floorGridMaterial";
                
        const entt::entity meshEntity = _scene->GetRegistry().create();
        
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
