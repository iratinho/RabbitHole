#pragma once

class Window;
class Scene;

struct InitializationParams;

class CameraSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(Scene* scene);

private:
//    void ComputeFirstPersonCamera(entt::registry& registry);
    void ComputeArcBallCamera(Scene* scene);

    Window* m_Window;
};
