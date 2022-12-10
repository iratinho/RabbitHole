#pragma once

namespace app {
    namespace window { class Window; }
    namespace renderer { class SimpleRendering; class RenderContext; class RenderSystem; }
    
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
        renderer::RenderContext* render_context_;
        renderer::SimpleRendering* simple_renderer_;
        renderer::RenderSystem* render_system_;
    };
}
