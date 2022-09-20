#include "application.h"
#include "window.h"
#include "renderer.h"

// system
#include <iostream>

// glfw3
#include "GLFW/glfw3.h"

namespace app {
    bool Application::Initialize() {
        if(!glfwInit()) {
            const int code = glfwGetError(nullptr);
            std::cerr << "[Error]: Failed to initialize glfw3 library. (Code: " <<  code << ")." << std::endl;
            return false;
        }

        constexpr window::InitializationParams window_params {
            "Vulkan",
            800,
            600
        };
        
        if(!main_window_.Initialize(window_params)) {
            std::cerr << "[Error]: Failed to initialize the main window." << std::endl;
            return false;
        }

        return true;
    }

    void Application::Shutdown() {
        glfwTerminate();        
    }

    void Application::Update() {
        while(!main_window_.ShouldWindowClose()) {
            main_window_.PoolEvents();
        }
    }
}
