#pragma once
#include "entt/entity/registry.hpp"

namespace app
{
    namespace window
    {
        class Window;
    }
}

struct InitializationParams;

class InputSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(entt::registry& registry);

private:
    app::window::Window* m_Window;
};
