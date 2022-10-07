#pragma once

// vulkan
#include "vulkan/vulkan_core.h"

namespace app::renderer {
    struct InitializationParams {
        bool validation_enabled_ = false; // Try to enable only in development
        uint32_t extension_count = 0;
        const char** instance_extensions = nullptr;
    };

    struct PhysicalDeviceInfo {
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceProperties device_properties;
        uint64_t score;
        int queue_family_index; // The queue family index that has Graphics and Compute operations
        std::vector<const char*> extensions;
        VkPhysicalDeviceFeatures features;
    };

    
    class Renderer {
    public:
        Renderer() = default;
        
        bool Initialize(const InitializationParams& initialization_params);

    private:
        bool CreateVulkanInstance();
        bool PickSuitableDevice();
        bool CreateLogicalDevice();

        InitializationParams initialization_params_;
        VkInstance instance_;
        PhysicalDeviceInfo device_info_;
        VkDevice logical_device_;
        uint32_t loader_version_;
    };
}

