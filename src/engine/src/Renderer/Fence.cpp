#include "Renderer/Fence.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKFence.hpp"
#endif

std::unique_ptr<Fence> Fence::MakeFence(const InitializationParams &params) {
    std::unique_ptr<Fence> instance;

#ifdef VULKAN_BACKEND
    instance = std::make_unique<VKFence>();
    instance->_params = params;
#endif
    
    if(instance) {
        instance->Initialize();
    }

    return instance;

}

void Fence::Initialize() {
}
