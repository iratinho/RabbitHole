#include "renderer.h"

// vulkan
#include "vulkan/vulkan_core.h"

namespace app::renderer {
    bool Renderer::Initialize(const InitializationParams& initialization_params) {
        return false;
    }

    VkInstance* Renderer::CreateVulkanInstance() {
        return instance_;
    }

}
