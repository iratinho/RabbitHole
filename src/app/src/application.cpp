#include "application.h"
#include "window.h"
#include "simple_rendering.h"

// system
#include <iostream>

// glfw3
#include <RenderSystem.h>

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

        // render_context_ = new renderer::RenderContext;
        // simple_renderer_ = new renderer::SimpleRendering;
        render_system_ = new renderer::RenderSystem;

        window::InitializationParams window_params {
            "Vulkan",
            800,
            600
        };

        window_params.resize_callback = &Application::HandleResize;
        window_params.callback_context = this;
        
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

        // render_context_->Initialize(renderer_params);

        if(!render_system_->Initialize(renderer_params)) {
            return false;
        }
        
        // if(!simple_renderer_->Initialize(render_context_, renderer_params)) {
        //     std::cerr << "[Error]: Failed to initialize the renderer system.." << std::endl;
        //     return false;
        // }

        return true;
    }

    void Application::Shutdown() {
        glfwTerminate();
        delete main_window_;
        // delete simple_renderer_;
    }

    void Application::Update() {
        while(main_window_ && !main_window_->ShouldWindowClose()) {
            main_window_->PoolEvents();
            render_system_->Process();
            // simple_renderer_->Draw();
        }
    }

    void Application::HandleResize(const void* callback_context, int width, int height) {
        const auto* app = static_cast<const Application*>(callback_context);
        if(app && app->render_system_) {
            app->render_system_->HandleResize(width, height);
        }

        // if(app && app->simple_renderer_) {
        //     // app->simple_renderer_->HandleResize(width, height);    
        // }
    }

}
