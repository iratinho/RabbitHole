#pragma once
#include "window.hpp"

class DesktopWindow : public Window {
public:
    bool Initialize(const WindowInitializationParams &params) override;
    void Shutdown() const override;
    void PoolEvents() override;
    void HideCursor() const override;
    void ShowCursor() const override;

    [[nodiscard]] bool ShouldWindowClose() const noexcept override;
    [[nodiscard]] glm::i32vec2 GetWindowSurfaceSize() const override;
    [[nodiscard]] std::tuple<std::uint32_t, const char **> GetRequiredExtensions() override;
    [[nodiscard]] void * GetWindow() const override;

protected:
    GLFWwindow* _window                         = nullptr;

private:
    static void DragDropCallback(GLFWwindow* window, int count, const char** paths);
    static void HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifier);
    static void HandleMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void HandleCursorCallback(GLFWwindow* window, double xpos, double ypos);
    static void HandleResizeCallback(GLFWwindow* window, int width, int height);
    static void HandleMouseWheelOffset(GLFWwindow* window, double xoffset, double yoffset);
};
