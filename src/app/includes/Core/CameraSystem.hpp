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

class CameraSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(Scene* scene);

private:
//    void ComputeFirstPersonCamera(entt::registry& registry);
    void ComputeArcBallCamera(Scene* scene);

    app::window::Window* m_Window;
};
