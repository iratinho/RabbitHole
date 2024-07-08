#include "Renderer/Fence.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKFence.hpp"
#endif

std::unique_ptr<Fence> Fence::MakeFence(const InitializationParams &params) {
    std::unique_ptr<Fence> instance;

#ifdef USING_VULKAN_API
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
