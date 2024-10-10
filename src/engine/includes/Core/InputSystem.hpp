#pragma once
class Window;
class Scene;

struct InitializationParams;

struct InputKeyAction {
    int scancode;
    int action;
    int modifier;
};

struct MouseButtonAction {
    int action;
    int mods;
};

class InputSystem {
public:
    InputSystem(Window* window);
    bool Process(Scene* scene);

private:
    void HandleInputKey(int key, int scancode, int action, int modifier);
    void HandleMouseButton(int button, int action, int mods);

private:
    Window* _window;

    // Tracks states of key/mouse events for the current input system
    // TODO can we merge it?
    std::map<int, InputKeyAction> _keyMap;
    std::map<int, MouseButtonAction> _mouseMap;
};
