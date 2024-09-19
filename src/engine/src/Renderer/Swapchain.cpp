#include "Renderer/Swapchain.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKSwapchain.hpp"
#endif

Swapchain::Swapchain(Device *device)
    : _device(device) {
}

std::unique_ptr<Swapchain> Swapchain::MakeSwapchain(Device* device) {
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKSwapchain>(device);
    return instance;
#endif

    return nullptr;
}

void Swapchain::MarkDirty() {
    _isDirty = true;
}
