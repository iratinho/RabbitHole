#pragma once

namespace app
{
    namespace window
    {
        class Window;
    }
}

class Scene;

struct InitializationParams;

class InputSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(Scene* scene);

private:
    app::window::Window* m_Window;
};
