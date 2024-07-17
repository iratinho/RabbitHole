#include "Renderer/RenderCommandEncoder.hpp"
#include "Renderer/render_context.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#endif

std::unique_ptr<RenderCommandEncoder> RenderCommandEncoder::MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext) { 
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKRenderCommandEncoder>();
    instance->_renderContext = renderContext;
    
    return instance;
#endif

    return nullptr;
}
