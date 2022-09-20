#pragma once

namespace app {
    namespace window { class Window; }

    class Application {
        public:
            bool Initialize();
            void Shutdown();
            void Update();

    private:
        window::Window main_window_;
    };
}
