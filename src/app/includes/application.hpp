#pragma once

class GeometryLoaderSystem;
class UISystem;
class InputSystem;
class CameraSystem;
class SimpleRendering;
class RenderContext;
class RenderSystem;
class Scene;

namespace app {
    namespace window { class Window; }
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
        
        window::Window* _mainWindow;
        RenderContext* render_context_;
        SimpleRendering* simple_renderer_;
        RenderSystem* _renderSystem;
        InputSystem* _inputSystem;
        CameraSystem* _cameraSystem;
        UISystem* _uiSystem;
        GeometryLoaderSystem* _geometryLoaderSystem;
        
        Scene* _scene;
    };
}
