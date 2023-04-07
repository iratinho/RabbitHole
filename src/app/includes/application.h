#pragma once
#include "entt/entt.hpp"

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
        window::Window* main_window_;
        RenderContext* render_context_;
        SimpleRendering* simple_renderer_;
        RenderSystem* render_system_;
        entt::registry registry;
    };
}
