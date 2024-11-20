#include "application.hpp"
#ifdef __EMSCRIPTEN__
#include "emscripten/emscripten.h"
#endif

int main() {
    app::Application app {};
    if(app.Initialize()) {
#ifdef __EMSCRIPTEN__
        auto callback = [](void *arg) {
            if(auto* app = static_cast<app::Application*>(arg)) {
                // ReSharper disable once CppExpressionWithoutSideEffects
                app->Update();
            }
        };

        emscripten_set_main_loop_arg(callback, &app, 0, true);
#else // __EMSCRIPTEN__
        bool bKeepUpdating = true;
        while (bKeepUpdating) {
            bKeepUpdating = app.Update();
        }
#endif
    }

    return 0;
}
