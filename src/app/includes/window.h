#pragma once

class GLFWwindow;

namespace app::window {
    enum WindowType {
        Normal,
        Splash,
        Hidden
    };

    struct FrameBufferSize {
        int width;
        int height;
    };
    
    typedef void (* DragDropCallbackFun)(const class Window* window, int path_count, const char* paths[]);
    typedef void (* KeyCallbackFun)(const class Window* window, int key, int scancode, int action, int mods);
    typedef void (* CursorCallbackFun)(const class Window* window, double xpos, double ypos);
    typedef void (* ResizeCallbackFun)(const void* callback_context, int width, int height);
    
    struct InitializationParams {
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
        
        bool Initialize(const InitializationParams& initialization_params) noexcept;
        bool ShouldWindowClose() const noexcept;
        void PoolEvents();
        std::tuple<std::uint32_t, const char**> GetRequiredExtensions();
        void* CreateSurface(void* instance);
        FrameBufferSize GetFramebufferSize();
        
    private:
        static void DragDropCallback(GLFWwindow* window, int count, const char** paths);
        static void HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifier);
        static void HandleCursorCallback(GLFWwindow* window, double xpos, double ypos);
        static void HandleResizeCallback(GLFWwindow* window, int width, int height);
        
        GLFWwindow* window_                         = nullptr;
        InitializationParams initialization_params_ = {};
    };
}
