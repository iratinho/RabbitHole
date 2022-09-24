#pragma once

// system
#include <cstdint>

// vulkan
#include "vulkan/vulkan_core.h"

namespace app::renderer {
    
    struct InitializationParams {
        bool validation_enabled_ = false; // Try to enable only in development
        uint32_t extension_count = 0;
        const char** instance_extensions = nullptr;
    };
    
    class Renderer {
    public:
        Renderer() = default;
        
        bool Initialize(const InitializationParams& initialization_params);

    private:
        bool CreateVulkanInstance();

        InitializationParams initialization_params_;
        VkInstance instance_;
    };    
}

