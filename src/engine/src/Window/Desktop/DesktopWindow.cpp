#include "Window/Desktop/DesktopWindow.hpp"
#include <GLFW/glfw3.h>

bool DesktopWindow::Initialize(const WindowInitializationParams &params) noexcept {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(params.width_, params.height_,
                               params.title_, nullptr, nullptr);

    // Sets the pointer to where glfw3 window callbacks will be invoked to
    glfwSetWindowUserPointer(window_, this);
    // Drag-Drop callback enabled
    glfwSetDropCallback(window_, DragDropCallback);
    // Key callback enabled
    glfwSetKeyCallback(window_, HandleKeyCallback);
    // Cursor callback enabled
    glfwSetCursorPosCallback(window_, HandleCursorCallback);
    // Resize callback
    glfwSetFramebufferSizeCallback(window_, HandleResizeCallback);
    // Mouse wheel scroll
    glfwSetScrollCallback(window_, HandleMouseWheelOffset);

    return Window::Initialize(params);
}

void DesktopWindow::Shutdown() const {
    Window::Shutdown();
}

void DesktopWindow::PoolEvents() {
    glfwPollEvents();

    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);

    // Get current mouse position
    const glm::vec2 currentMousePos(xpos, ypos);

    // Calculate mouse delta
    const glm::vec2 mouseDelta = currentMousePos - glm::vec2(m_MouseDelta.z, m_MouseDelta.w);

    // Store current mouse position as previous mouse position
    m_MouseDelta.z = xpos;
    m_MouseDelta.w = ypos;

    // Output mouse delta
    // std::cout << "Mouse Delta X: " << mouseDelta.x << " | Mouse Delta Y: " << mouseDelta.y << std::endl;

    // Store mouse delta
    m_MouseDelta.x = mouseDelta.x;
    m_MouseDelta.y = mouseDelta.y;
}

void DesktopWindow::HideCursor() const {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void DesktopWindow::ShowCursor() const {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool DesktopWindow::ShouldWindowClose() const noexcept {
    return glfwWindowShouldClose(window_) && Window::ShouldWindowClose();
}

glm::i32vec2 DesktopWindow::GetWindowSurfaceSize() const {
    glm::i32vec2 size;
    glfwGetFramebufferSize(window_, &size.x, &size.y);

    return size;
}

std::tuple<std::uint32_t, const char **> DesktopWindow::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    return std::make_tuple(glfwExtensionCount, extensions);
}

void * DesktopWindow::GetWindow() const {
    return window_;
}

void DesktopWindow::DragDropCallback(GLFWwindow* window, int count, const char** paths) {
    if(const auto instance = static_cast<DesktopWindow*>(glfwGetWindowUserPointer(window))) {
        instance->GetDragDropDelegate().Broadcast(count, paths);
    }
}

void DesktopWindow::HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifier) {
    if(const auto instance = static_cast<DesktopWindow*>(glfwGetWindowUserPointer(window))) {
        instance->GetKeyDelegate().Broadcast(key, scancode, action, modifier);
    }
}

void DesktopWindow::HandleCursorCallback(GLFWwindow* window, double xpos, double ypos) {
    if(const auto instance = static_cast<DesktopWindow*>(glfwGetWindowUserPointer(window))) {
        instance->GetMousePosDelegate().Broadcast({(float)xpos, (float)ypos});
    }
}

void DesktopWindow::HandleResizeCallback(GLFWwindow* window, int width, int height) {
    if(const auto instance = static_cast<DesktopWindow*>(glfwGetWindowUserPointer(window))) {
        instance->GetWindowResizeDelegate().Broadcast({width, height});
    }
}

void DesktopWindow::HandleMouseWheelOffset(GLFWwindow* window, double xoffset, double yoffset)
{
    if(const auto instance = static_cast<DesktopWindow*>(glfwGetWindowUserPointer(window))) {
        // Update offsets internally
        instance->m_CurrentMouseDelta.x = xoffset;
        instance->m_CurrentMouseDelta.y = yoffset;
    }
}
