#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUBlitCommandEncoder.hpp"
#endif

std::unique_ptr<BlitCommandEncoder> BlitCommandEncoder::MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device) {
#ifdef VULKAN_BACKEND
    auto instance = std::make_unique<VKBlitCommandEncoder>(commandBuffer, graphicsContext, device);
    return instance;
#else
    auto instance = std::make_unique<WebGPUBlitCommandEncoder>(commandBuffer, graphicsContext, device);
    return instance;
#endif

    return nullptr;
}
