#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#endif

std::unique_ptr<RenderCommandEncoder> RenderCommandEncoder::MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device) {
#ifdef USING_VULKAN_API
    return std::make_unique<VKRenderCommandEncoder>(commandBuffer, graphicsContext, device);
#endif

    return nullptr;
}
