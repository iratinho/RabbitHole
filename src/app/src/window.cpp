#include "window.h"

// system
#include <functional>

// glfw3
#include <GLFW/glfw3.h>

namespace app::window {
    bool Window::Initialize(const InitializationParams& initialization_params) noexcept {
        window_ = glfwCreateWindow(initialization_params.width_, initialization_params.height_, initialization_params.title_, nullptr, nullptr);
        // Sets the pointer to where glfw3 window callbacks will be invoked to
        glfwSetWindowUserPointer(window_, this);
        // Drag-Drop callback enabled
        glfwSetDropCallback(window_, DragDropCallback);
        // Key callback enabled
        glfwSetKeyCallback(window_, HandleKeyCallback);
        // Cursor callback enabled
        glfwSetCursorPosCallback(window_, HandleCursorCallback);

        return false;        
    }

    bool Window::ShouldWindowClose() const noexcept {
        return glfwWindowShouldClose(window_);        
    }

    void Window::PoolEvents() {
        glfwPollEvents();
    }
    
    void Window::DragDropCallback(GLFWwindow* window, int count, const char** paths) {
        const Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.drag_drop_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.drag_drop_callback, window_instance, count, paths);
        }
    }

    void Window::HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifier) {
        const Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.drag_drop_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.key_callback, window_instance, key, scancode, action, modifier);
        }
    }

    void Window::HandleCursorCallback(GLFWwindow* window, double xpos, double ypos) {
        const Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(window_instance && window_instance->initialization_params_.drag_drop_callback != nullptr) {
            std::invoke(window_instance->initialization_params_.cursor_callback, window_instance, xpos, ypos);
        }
    }
}
