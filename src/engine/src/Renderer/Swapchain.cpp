#include "Renderer/Swapchain.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKSwapchain.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUSwapchain.hpp"
#endif

Swapchain::Swapchain(Device *device)
    : _device(device) {
}

std::unique_ptr<Swapchain> Swapchain::MakeSwapchain(Device* device) {
#ifdef VULKAN_BACKEND
    auto instance = std::make_unique<VKSwapchain>(device);
    return instance;
#else
    auto instance = std::make_unique<WebGPUSwapchain>(device);
    return instance;
#endif

    return nullptr;
}

void Swapchain::MarkDirty() {
    _isDirty = true;
}
