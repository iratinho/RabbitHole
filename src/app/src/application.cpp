#include "application.h"
#include <window.h>

// system
#include <iostream>

// glfw3
#include "GLFW/glfw3.h"

namespace app {
    namespace internals {
        window::Window main_window_;    
    }
    
    bool Application::Initialize() {
        if(!glfwInit()) {
            const int code = glfwGetError(nullptr);
            std::cerr << "[Error]: Failed to initialize glfw3 library. (Code: " <<  code << ")." << std::endl;
            return false;
        }

        constexpr app::window::InitializationParams window_params {
            "Vulkan",
            800,
            600
        };
        
        if(internals::main_window_.Initialize(window_params)) {
            std::cerr << "[Error]: Failed to initialize the main window." << std::endl;
            return false;
        }

        return true;
    }

    void Application::Shutdown() {
        glfwTerminate();        
    }

    void Application::Update() {
        while(!internals::main_window_.ShouldWindowClose()) {
            // Do Updates
        }
    }    
}
