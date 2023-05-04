#include "window.h"

// system
#include <functional>

// glfw3
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

namespace app::window {
    bool Window::Initialize(const InitializationParams& initialization_params) noexcept {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(initialization_params.width_, initialization_params.height_, initialization_params.title_, nullptr, nullptr);
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
        
        initialization_params_ = initialization_params;
        
        return true;        
    }

    bool Window::ShouldWindowClose() const noexcept {
        return glfwWindowShouldClose(window_);        
    }

    void Window::PoolEvents() {
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

    void Window::ClearDeltas()
    {
        m_CurrentMouseDelta = glm::vec2(0.0f);
    }

    std::tuple<uint32_t, const char**> Window::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        return std::make_tuple(glfwExtensionCount, extensions);
    }
    
    void Window::DragDropCallback(GLFWwindow* window, int count, const char** paths) {
        const Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.drag_drop_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.drag_drop_callback, window_instance, count, paths);
        }
    }

    void Window::HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifier) {
        const Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.key_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.key_callback, window_instance, key, scancode, action, modifier);
        }
    }

    void Window::HandleCursorCallback(GLFWwindow* window, double xpos, double ypos) {
        Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        
        if(window_instance && window_instance->initialization_params_.cursor_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.cursor_callback, window_instance, xpos, ypos);
        }
    }

    void Window::HandleResizeCallback(GLFWwindow* window, int width, int height) {
        const Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.resize_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.resize_callback, window_instance->initialization_params_.callback_context, width, height);
        }
    }

    void Window::HandleMouseWheelOffset(GLFWwindow* window, double xoffset, double yoffset)
    {
        Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.resize_callback != nullptr) {
            // TODO
        }

        if(window_instance) {
            // Update offsets internally
            window_instance->m_CurrentMouseDelta.x = xoffset;
            window_instance->m_CurrentMouseDelta.y = yoffset;
        }
    }
    
    void* Window::CreateSurface(void* instance)
    {
        VkSurfaceKHR surface;
        VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(instance), window_, nullptr, &surface);

        return surface;
    }

    FrameBufferSize Window::GetFramebufferSize() {
        FrameBufferSize framebuffer_size {};
        glfwGetFramebufferSize(window_, &framebuffer_size.width, &framebuffer_size.height);

        return framebuffer_size;
    }

    void Window::HideCursor() {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Window::ShowCursor() {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
