#pragma once
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "Delegates.h"

#include "Core/InputSystem.hpp"
#include "Renderer/Device.hpp"


class GLFWwindow;

enum WindowType {
    Normal,
    Splash,
    Hidden
};

struct FrameBufferSize {
    int width;
    int height;
};

struct WindowInitializationParams {
    const char* title_                      = "Untitled";
    std::uint32_t width_                    = 800;
    std::uint32_t height_                   = 600;
    WindowType window_type_                 = Normal;
};
    
class Window {
public:
    Window() = default;
    
    ~Window() {
        Shutdown();
    };
        
    bool Initialize(const WindowInitializationParams& initialization_params) noexcept;
    void Shutdown() const;
    bool ShouldWindowClose() const noexcept;
    void PoolEvents();
    void ClearDeltas();

    static std::tuple<std::uint32_t, const char**> GetRequiredExtensions();
    void* CreateSurface(void* instance);
    [[nodiscard]] glm::i32vec2 GetWindowSurfaceSize() const;
    [[nodiscard]] GLFWwindow* GetWindow() const { return window_; }
    [[nodiscard]] glm::vec2 GetMouseDelta() const { return {m_MouseDelta.x, m_MouseDelta.y}; }
    [[nodiscard]] glm::vec2 GetMouseWheelDelta() const { return m_CurrentMouseDelta; }

    void HideCursor() const;
    void ShowCursor() const;
    
    [[nodiscard]] Device* GetDevice() const { return _device.get(); };
    [[nodiscard]] InputSystem* GetInputSystem() const { return _inputSystem.get(); }

    [[nodiscard]] MulticastDelegate<glm::i32vec2>& GetWindowResizeDelegate() { return _windowResizeDelegate; }
    [[nodiscard]] MulticastDelegate<glm::vec2>& GetMousePosDelegate() { return _mouseMoveDelegate; }
    [[nodiscard]] MulticastDelegate<int, const char**>& GetDragDropDelegate() { return _dragDropDelegate; }
    [[nodiscard]] MulticastDelegate<int, int, int, int>& GetKeyDelegate() { return _keyDelegate; }
private:
    static void DragDropCallback(GLFWwindow* window, int count, const char** paths);
    static void HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifier);
    static void HandleCursorCallback(GLFWwindow* window, double xpos, double ypos);
    static void HandleResizeCallback(GLFWwindow* window, int width, int height);
    static void HandleMouseWheelOffset(GLFWwindow* window, double xoffset, double yoffset);
        
    GLFWwindow* window_                         = nullptr;
    WindowInitializationParams initialization_params_ = {};

    // First vec2 is mouse delta and then last mouse pos XY, XY
    glm::vec4 m_MouseDelta = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec2 m_CurrentMouseDelta = glm::vec2(0.0f);
        
    std::unique_ptr<Device> _device;
    std::unique_ptr<InputSystem> _inputSystem;

    MulticastDelegate<glm::i32vec2> _windowResizeDelegate;
    MulticastDelegate<glm::vec2> _mouseMoveDelegate;
    MulticastDelegate<int, const char**> _dragDropDelegate;
    MulticastDelegate<int, int, int, int> _keyDelegate;

};

