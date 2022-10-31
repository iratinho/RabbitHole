#pragma once

// vulkan
#include "vulkan/vulkan_core.h"

namespace app::window {
    class Window;
}

namespace app::renderer {
    struct InitializationParams {
        bool validation_enabled_ = false; // Try to enable only in development
        uint32_t extension_count = 0;
        const char** instance_extensions = nullptr;
        app::window::Window* window_ = nullptr;
    };

    struct PhysicalDeviceInfo {
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceProperties device_properties;
        uint64_t score;
        uint32_t queue_family_index; // The queue family index that has Graphics and Compute operations
        uint32_t graphics_queue_family_index;
        uint32_t compute_queue_family_index;
        uint32_t presentation_queue_family_index;
        std::vector<const char*> extensions;
        VkPhysicalDeviceFeatures features;
        VkQueue graphics_queue;
        VkQueue present_queue;
    };
    
    class RenderContext {
    public:
        RenderContext() = default;
        bool Initialize(const InitializationParams& initialization_params);
        bool CreateShader(const char* shader_path, VkShaderStageFlagBits shader_stage, VkPipelineShaderStageCreateInfo& shader_stage_create_info);
        bool RecreateSwapchain();
        VkDevice GetLogicalDeviceHandle() { return logical_device_; }
        VkPhysicalDevice GetPhysicalDeviceHandle() { return device_info_.physical_device; }
        uint32_t GetGraphicsQueueIndex() { return device_info_.graphics_queue_family_index; }
        VkQueue GetGraphicsQueueHandle() { return device_info_.graphics_queue; }
        VkQueue GetPresentQueueHandle() { return device_info_.present_queue; }
        VkSwapchainKHR GetSwapchainHandle() const { return swapchain_ ;}
        VkExtent2D GetSwapchainExtent() const;
        int GetSwapchainImageCount() { return 2; }// Hardcoded for now
        std::vector<VkImageView>& GetSwapchainImageVies() { return swapchain_image_views_; }
        
    private:
        bool CreateVulkanInstance();
        bool PickSuitableDevice();
        bool CreateLogicalDevice();
        bool CreateWindowSurface();
        bool CreateSwapChain();

        InitializationParams initialization_params_;
        VkInstance instance_;
        uint32_t loader_version_;
        PhysicalDeviceInfo device_info_;
        VkDevice logical_device_;
        VkSurfaceKHR surface_;
        VkSwapchainKHR swapchain_;
        std::vector<VkImageView> swapchain_image_views_;
    };
}

