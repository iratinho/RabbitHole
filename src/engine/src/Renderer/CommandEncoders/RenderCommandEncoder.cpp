#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/render_context.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#endif

std::unique_ptr<RenderCommandEncoder> RenderCommandEncoder::MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, RenderContext* renderContext) {
#ifdef USING_VULKAN_API
    return std::make_unique<VKRenderCommandEncoder>(commandBuffer, graphicsContext, renderContext);
#endif

    return nullptr;
}
