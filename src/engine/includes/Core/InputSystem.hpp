#pragma once

class Window;
class Scene;

struct InitializationParams;

class InputSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(Scene* scene);

private:
    Window* m_Window;
};
