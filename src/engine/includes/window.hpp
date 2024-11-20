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
    
    virtual ~Window() {
        Window::Shutdown();
    };

    static std::unique_ptr<Window> MakeWindow();
        
    virtual bool Initialize(const WindowInitializationParams& params);
    virtual void Shutdown() const;
    virtual void* CreateSurface(void* instance);
    virtual void PoolEvents();
    virtual void HideCursor() const;
    virtual void ShowCursor() const;

    [[nodiscard]] virtual bool ShouldWindowClose() const noexcept;
    [[nodiscard]] virtual glm::i32vec2 GetWindowSurfaceSize() const;
    [[nodiscard]] virtual std::tuple<std::uint32_t, const char**> GetRequiredExtensions();
    [[nodiscard]] virtual void* GetWindow() const;

    void ClearDeltas();
    [[nodiscard]] glm::vec2 GetMouseDelta() const { return {m_MouseDelta.x, m_MouseDelta.y}; }
    [[nodiscard]] glm::vec2 GetMouseWheelDelta() const { return m_CurrentMouseDelta; }
    [[nodiscard]] Device* GetDevice() const { return _device.get(); };
    [[nodiscard]] InputSystem* GetInputSystem() const { return _inputSystem.get(); }
    [[nodiscard]] MulticastDelegate<glm::i32vec2>& GetWindowResizeDelegate() { return _windowResizeDelegate; }
    [[nodiscard]] MulticastDelegate<glm::vec2>& GetMousePosDelegate() { return _mouseMoveDelegate; }
    [[nodiscard]] MulticastDelegate<int, const char**>& GetDragDropDelegate() { return _dragDropDelegate; }
    [[nodiscard]] MulticastDelegate<int, int, int, int>& GetKeyDelegate() { return _keyDelegate; }
    [[nodiscard]] MulticastDelegate<int, int, int>& GetMouseButtonDelegate() { return _mouseButtonDelegate; }


protected:
    // First vec2 is mouse delta and then last mouse pos XY, XY
    glm::vec4 m_MouseDelta = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec2 m_CurrentMouseDelta = glm::vec2(0.0f);

    WindowInitializationParams _params = {};

private:

    std::unique_ptr<Device> _device;
    std::unique_ptr<InputSystem> _inputSystem;

    MulticastDelegate<glm::i32vec2> _windowResizeDelegate;
    MulticastDelegate<glm::vec2> _mouseMoveDelegate;
    MulticastDelegate<int, const char**> _dragDropDelegate;
    MulticastDelegate<int, int, int, int> _keyDelegate;
    MulticastDelegate<int, int, int> _mouseButtonDelegate;

};

