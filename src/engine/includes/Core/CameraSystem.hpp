#pragma once

class Window;
class Scene;

struct InitializationParams;

class CameraSystem {
public:
    bool Initialize(Window* window);
    bool Process(Scene* scene);

private:
//    void ComputeFirstPersonCamera(entt::registry& registry);
    void ComputeArcBallCamera(Scene* scene) const;
    void Pan();

    Window* m_Window;
};
