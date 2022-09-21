#pragma once

namespace app::renderer {
    typedef struct VkInstance_T* VkInstance;
    
    struct InitializationParams {
        bool validation_enabled_ = false; // Try to enable only in development
    };
    
    class Renderer {
    public:
        Renderer() = default;
        
        bool Initialize(const InitializationParams& initialization_params);

    private:
        VkInstance* CreateVulkanInstance();
        VkInstance* instance_;
    };    
}

