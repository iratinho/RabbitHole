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

class CameraSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(entt::registry& registry);

private:
    void ComputeFirstPersonCamera(entt::registry& registry);
    void ComputeArcBallCamera(entt::registry& registry);

    app::window::Window* m_Window;
};
