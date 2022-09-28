#pragma once

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
        bool PickSuitableDevice();

        InitializationParams initialization_params_;
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        uint32_t loader_version_;
    };    
}

