#pragma once

namespace app {
    namespace window { class Window; }
    namespace renderer { class SimpleRendering; }
    
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
        renderer::SimpleRendering* simple_renderer_;
    };
}
