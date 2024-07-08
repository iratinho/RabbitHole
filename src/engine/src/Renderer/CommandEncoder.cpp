#include "Renderer/CommandEncoder.hpp"
#include "Renderer/render_context.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKCommandEncoder.hpp"
#endif

std::unique_ptr<CommandEncoder> CommandEncoder::MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext) { 
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKCommandEncoder>();
    instance->_renderContext = renderContext;
    
    return instance;
#endif

    return nullptr;
}
