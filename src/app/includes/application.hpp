#pragma once
#include "entt/entt.hpp"

class GeometryLoaderSystem;
class UISystem;
class InputSystem;
class CameraSystem;
class SimpleRendering;
class RenderContext;
class RenderSystem;


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
        static void HandleResize(const void* callback_context, int width, int height);
        static void HandleDragAndDrop(const void* callback_context, int count, const char** paths);
        window::Window* main_window_;
        RenderContext* render_context_;
        SimpleRendering* simple_renderer_;
        RenderSystem* render_system_;
        InputSystem* m_InputSystem;
        CameraSystem* m_CameraSystem;
        UISystem* m_UISystem;
        GeometryLoaderSystem* _geometryLoaderSystem;
        entt::registry registry;
    };
}
