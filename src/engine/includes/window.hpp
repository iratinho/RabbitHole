#pragma once
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

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
    
typedef void (* DragDropCallbackFun)(const void* callback_context, int path_count, const char** paths);
typedef void (* KeyCallbackFun)(const class Window* window, int key, int scancode, int action, int mods);
typedef void (* CursorCallbackFun)(const class Window* window, double xpos, double ypos);
typedef void (* ResizeCallbackFun)(const void* callback_context, int width, int height);
    
struct WindowInitializationParams {
    const char* title_                      = "Untitled";
    std::uint32_t width_                    = 800;
    std::uint32_t height_                   = 600;
    WindowType window_type_                 = Normal;
    DragDropCallbackFun drag_drop_callback  = nullptr;
    KeyCallbackFun key_callback             = nullptr;
    CursorCallbackFun cursor_callback       = nullptr;
    ResizeCallbackFun resize_callback       = nullptr;
    void* callback_context                  = nullptr;
};
    
class Window {
public:
    Window() = default;
    
    ~Window() {
        Shutdown();
    };
        
    bool Initialize(const WindowInitializationParams& initialization_params) noexcept;
    void Shutdown();
    bool ShouldWindowClose() const noexcept;
    void PoolEvents();
    void ClearDeltas();
    std::tuple<std::uint32_t, const char**> GetRequiredExtensions();
    void* CreateSurface(void* instance);
    glm::i32vec2 GetWindowSurfaceSize();
    GLFWwindow* GetWindow() { return window_; }
    const glm::vec2 GetMouseDelta() const  { return glm::vec2(m_MouseDelta.x, m_MouseDelta.y); }
    const glm::vec2 GetMouseWheelDelta() const { return m_CurrentMouseDelta; }

    void HideCursor();
    void ShowCursor();
    
    Device* GetDevice() { return _device.get(); };
        
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
};

