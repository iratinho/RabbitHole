#pragma once
//#define VK_USE_PLATFORM_WIN32_KHR
//#define VK_NO_PROTOTYPES
#include "vulkan/vulkan_core.h"

// Global mapping for vulkan functions that are loaded with the VulkanLoader
namespace VkFunc {
#define EXPORTED_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) extern PFN_##name name; 
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) extern PFN_##name name; 

#include "VulkanFunctions.inl"
}

class VulkanLoader {
public:
    bool Initialize();
    bool LoadInstanceLevelFunctions(VkInstance instance);
    bool LoadInstanceExtensionLevelFunctions( VkInstance instance, const std::vector<const char*>& instance_extensions);
    bool LoadDeviceLevelFunctions(VkDevice device);
    bool LoadDeviceExtensionsLevelFunctions(VkDevice device, const std::vector<const char*>& device_extensions);

private:
    void LoadModule();
    void* _vulkan_module;
};

