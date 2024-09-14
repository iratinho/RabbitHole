#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"
#endif

std::unique_ptr<BlitCommandEncoder> BlitCommandEncoder::MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, RenderContext* renderContext) {
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKBlitCommandEncoder>(commandBuffer, graphicsContext, renderContext);
    return instance;
#endif

    return nullptr;
}
