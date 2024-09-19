#pragma once

class Window;
class Scene;

struct InitializationParams;

class InputSystem {
public:
    InputSystem(Window* window);
    bool Process(Scene* scene) const;

private:
    Window* _window;
};
