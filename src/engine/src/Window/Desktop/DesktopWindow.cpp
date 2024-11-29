#include "Window/Desktop/DesktopWindow.hpp"
#include <GLFW/glfw3.h>

bool DesktopWindow::Initialize(const WindowInitializationParams &params) {
    if(!glfwInit()) {
#ifndef __EMSCRIPTEN__ 
        const int code = glfwGetError(nullptr);
        throw std::runtime_error("[Error]: Failed to initialize glfw3 library. (Code: " + std::to_string(code) + ").");
#endif
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(params.width_, params.height_,
                               params.title_, nullptr, nullptr);

    // Sets the pointer to where glfw3 window callbacks will be invoked to
    glfwSetWindowUserPointer(_window, this);
    // Drag-Drop callback enabled
    glfwSetDropCallback(_window, DragDropCallback);
    // Key callback enabled
    glfwSetKeyCallback(_window, HandleKeyCallback);
    // Key callback for mouse events
    glfwSetMouseButtonCallback(_window, HandleMouseButtonCallback);
    // Cursor callback enabled
    glfwSetCursorPosCallback(_window, HandleCursorCallback);
    // Resize callback
    glfwSetFramebufferSizeCallback(_window, HandleResizeCallback);
    // Mouse wheel scroll
    glfwSetScrollCallback(_window, HandleMouseWheelOffset);

    return Window::Initialize(params);
}

void DesktopWindow::Shutdown() const {
    glfwTerminate();
    Window::Shutdown();
}

void DesktopWindow::PoolEvents() {
    glfwPollEvents();

    double xpos, ypos;
    glfwGetCursorPos(_window, &xpos, &ypos);

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
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void DesktopWindow::ShowCursor() const {
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool DesktopWindow::ShouldWindowClose() const noexcept {
    return glfwWindowShouldClose(_window) && Window::ShouldWindowClose();
}

glm::i32vec2 DesktopWindow::GetWindowSurfaceSize() const {
    glm::i32vec2 size;
    glfwGetFramebufferSize(_window, &size.x, &size.y);
    
    //glfwGetWindowSize(_window, &size.x, &size.y); // Works in webgpu does not work in vk

    return size;
}

std::tuple<std::uint32_t, const char **> DesktopWindow::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    return std::make_tuple(glfwExtensionCount, extensions);
}

void * DesktopWindow::GetWindow() const {
    return _window;
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

void DesktopWindow::HandleMouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if(const auto instance = static_cast<DesktopWindow*>(glfwGetWindowUserPointer(window))) {
        instance->GetMouseButtonDelegate().Broadcast(button, action, mods);
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
