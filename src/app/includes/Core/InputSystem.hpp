#pragma once
#include "entt/entity/registry.hpp"

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
    bool Process(Scene* scene, entt::registry& registry);

private:
    app::window::Window* m_Window;
};
