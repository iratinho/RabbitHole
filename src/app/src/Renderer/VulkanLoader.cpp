#include "Renderer/VulkanLoader.hpp"


#if defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#define HMODULE 
#define LoadProcAddress(ptr, dllname) dlsym(ptr, dllname)
#define LoadLib(name) dlopen(name, RTLD_NOW)
#elif defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#define LoadProcAddress(ptr, dllname) GetProcAddress((HMODULE)ptr, dllname)
#define LoadLib(name) LoadLibrary(name)
#endif

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
VkFunc::name = (PFN_##name)LoadProcAddress( _vulkan_module, #name );      \
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

#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) \
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
#ifdef __unix__
    const char* vkLibrary = "libvulkan.so";
#elif defined(__APPLE__)
    const char* vkLibrary = _GLFW_VULKAN_LIBRARY;
#elif defined(_WIN32) || defined(WIN32)
    const char* vkLibrary = "vulkan-1.dll";
#endif
    _vulkan_module = LoadLib(vkLibrary);
}

