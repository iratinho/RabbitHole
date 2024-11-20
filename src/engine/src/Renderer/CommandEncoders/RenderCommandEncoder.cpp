#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/CommandBuffer.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPURenderCommandEncoder.hpp"
#endif

std::unique_ptr<RenderCommandEncoder> RenderCommandEncoder::MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device) {
#ifdef VULKAN_BACKEND
    return std::make_unique<VKRenderCommandEncoder>(commandBuffer, graphicsContext, device);
#else
    return std::make_unique<WebGPURenderCommandEncoder>(commandBuffer, graphicsContext, device);
#endif

    return nullptr;
}
