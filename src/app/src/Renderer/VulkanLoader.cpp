#include "Renderer/VulkanLoader.h"
#include "Windows.h"

// #if defined(VK_USE_PLATFORM_WIN32_KHR)
#define LoadProcAddress GetProcAddress
// #elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
// #define LoadProcAddress dlsym
// #endif

namespace VkFunc
{
#define EXPORTED_VULKAN_FUNCTION( name ) PFN_##name name; 
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name; 
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name; 
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name; 
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) PFN_##name name; 

#include "Renderer/VulkanFunctions.inl"
}

bool VulkanLoader::Initialize()
{
    LoadModule();

#define EXPORTED_VULKAN_FUNCTION( name )                       \
VkFunc::name = (PFN_##name)LoadProcAddress( (HMODULE)_vulkan_module, #name );      \
if( VkFunc::name == nullptr ) {                                        \
std::cout << "Could not load exported Vulkan function named: " \
#name << std::endl;                                            \
return false;                                                  \
}

#define GLOBAL_LEVEL_VULKAN_FUNCTION( name )                   \
VkFunc::name = (PFN_##name)VkFunc::vkGetInstanceProcAddr( nullptr, #name );    \
if( VkFunc::name == nullptr ) {                                        \
std::cout << "Could not load global-level function named: "    \
#name << std::endl;                                            \
return false;                                                  \
} 

#include "Renderer/VulkanFunctions.inl"
    
    return true;
}

bool VulkanLoader::LoadInstanceLevelFunctions(VkInstance instance) {
    if(instance) {
        
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) \
VkFunc::name = (PFN_##name)VkFunc::vkGetInstanceProcAddr( instance, #name ); \
if( (VkFunc::name) == nullptr ) { \
std::cout << "Could not load instance-level Vulkan function named: " \
#name << std::endl; \
return false; \
}

#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name ) \
VkFunc::name = (PFN_##name)VkFunc::vkGetInstanceProcAddr( instance, #name ); \
if( (VkFunc::name) == nullptr ) { \
std::cout << "Could not load instance-level Vulkan function named: " \
#name << std::endl; \
return false; \
}
#include "Renderer/VulkanFunctions.inl" 

        return true;
     }
    return false;
}

bool VulkanLoader::LoadInstanceExtensionLevelFunctions(VkInstance instance, const std::vector<const char*>& instance_extensions)
{
    if(instance && instance_extensions.size() > 0)
    {
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) \
        for( auto & enabled_extension : instance_extensions ) \
        { \
            if( std::string( enabled_extension ) == std::string( extension )) \
            { \
                VkFunc::name = (PFN_##name)VkFunc::vkGetInstanceProcAddr( instance, #name ); \
                if( (VkFunc::name) == nullptr ) { \
                    std::cout << "Could not load instance-level Vulkan function named: " #name << std::endl; \
                    return false; \
                } \
            } \
        }

#include "Renderer/VulkanFunctions.inl" 
        return true;
    }
    
    return false;
}

bool VulkanLoader::LoadDeviceLevelFunctions(VkDevice device) {
    if(device) {
        
#define DEVICE_LEVEL_VULKAN_FUNCTION( name )                                                \
VkFunc::name = (PFN_##name)VkFunc::vkGetDeviceProcAddr( device, #name );                                    \
if( VkFunc::name == nullptr ) {                                                                     \
std::cout << "Could not load device-level Vulkan function named: " #name << std::endl;      \
return false;                                                                               \
} 
#include "Renderer/VulkanFunctions.inl"
        
        return true;
    }
    return false;
}

bool VulkanLoader::LoadDeviceExtensionsLevelFunctions(VkDevice device, const std::vector<const char*>& device_extensions)
{
    if(device && device_extensions.size() > 0)
    {
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) \
        for( auto & enabled_extension : device_extensions ) \
        { \
            if( std::string( enabled_extension ) == std::string( extension )) \
            { \
                VkFunc::name = (PFN_##name)VkFunc::vkGetDeviceProcAddr( device, #name ); \
                if( (VkFunc::name) == nullptr ) { \
                    std::cout << "Could not load device-level Vulkan function named: " #name << std::endl; \
                    return false; \
                } \
            } \
        }

#include "Renderer/VulkanFunctions.inl" 
        return true;
    }
    
    return false;

}

void VulkanLoader::LoadModule()
{
    // TODO Other platforms
    _vulkan_module = LoadLibrary("vulkan-1.dll");
}

