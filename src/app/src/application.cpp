#include "application.h"
#include "window.h"
#include "simple_rendering.h"

// system
#include <iostream>

// glfw3
#include "GLFW/glfw3.h"

namespace app {
    Application::~Application() {
        Shutdown();
    }
    
    bool Application::Initialize() {
        if(!glfwInit()) {
            const int code = glfwGetError(nullptr);
            std::cerr << "[Error]: Failed to initialize glfw3 library. (Code: " <<  code << ")." << std::endl;
            return false;
        }

        main_window_ = new window::Window;
        simple_renderer_ = new renderer::SimpleRendering;

        constexpr window::InitializationParams window_params {
            "Vulkan",
            800,
            600
        };
        
        if(!main_window_->Initialize(window_params)) {
            std::cerr << "[Error]: Failed to initialize the main window." << std::endl;
            return false;
        }

        const auto&[extensionCount, extensions] = main_window_->GetRequiredExtensions();
        
        const renderer::InitializationParams renderer_params {
            true,
            extensionCount,
            extensions,
            main_window_
        };
        
        if(!simple_renderer_->Initialize(renderer_params)) {
            std::cerr << "[Error]: Failed to initialize the renderer system.." << std::endl;
            return false;
        }

        return true;
    }

    void Application::Shutdown() {
        glfwTerminate();
        delete main_window_;
        delete simple_renderer_;
    }

    void Application::Update() {
        while(main_window_ && !main_window_->ShouldWindowClose()) {
            main_window_->PoolEvents();
            simple_renderer_->Draw();
        }
    }
}
