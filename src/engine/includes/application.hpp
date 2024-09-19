#pragma once

class GeometryLoaderSystem;
class UISystem;
class InputSystem;
class CameraSystem;
class SimpleRendering;
class RenderContext;
class RenderSystemV2;
class Scene;
class Window;

namespace app {
    class Application {
        public:
            Application() = default;
            ~Application();
        
            bool Initialize();
            void Shutdown();
            void Update();

    private:
        void CreateDefaultCamera();
        void CreateDefaultLights();
        void CreateFloorGridMesh();
        static void HandleResize(const void* callback_context, int width, int height);
        static void HandleDragAndDrop(const void* callback_context, int count, const char** paths);
        
        Window* _mainWindow;
        RenderContext* render_context_;
        SimpleRendering* simple_renderer_;
        RenderSystemV2* _renderSystem;
        CameraSystem* _cameraSystem;
        UISystem* _uiSystem;
        GeometryLoaderSystem* _geometryLoaderSystem;
        
        Scene* _scene;
    };
}
