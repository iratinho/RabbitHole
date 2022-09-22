#include "renderer.h"

// vulkan
#include <iostream>

#include "vulkan/vulkan_core.h"

#define VULKAN_DESIRED_VERSION VK_MAKE_VERSION(1, 3, 201)

namespace app::renderer {
    bool Renderer::Initialize(const InitializationParams& initialization_params) {
        bool b_initialized = false;
        b_initialized = CreateVulkanInstance();

        return b_initialized;
    }

    bool Renderer::CreateVulkanInstance() {
        uint32_t loader_version;
        if(vkEnumerateInstanceVersion(&loader_version) != VK_SUCCESS) {
            std::cerr << "Unable to get the vulkan loader version." << std::endl;
            return false;
        }

        std::cout << std::printf("Current vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(loader_version), VK_VERSION_MINOR(loader_version), VK_VERSION_PATCH(loader_version)) << std::endl;
        std::cout << std::printf("Desired vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(VULKAN_DESIRED_VERSION), VK_VERSION_MINOR(VULKAN_DESIRED_VERSION), VK_VERSION_PATCH(VULKAN_DESIRED_VERSION)) << std::endl;;

        // The loader version installed in the running system is lower than our application target loader
        if(VULKAN_DESIRED_VERSION > loader_version) {
            std::cerr << "Unable to create vulkan instance, the current installed vulkan loader does not meet the requirements for this application.\nPlease update your vulkan installation." << std::endl;
            return false;
        }
        
        return false; // for now on purpose
    }
}
