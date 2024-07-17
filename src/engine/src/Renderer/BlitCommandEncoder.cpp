#include "Renderer/BlitCommandEncoder.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"
#endif

std::unique_ptr<BlitCommandEncoder> BlitCommandEncoder::MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext) {
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKBlitCommandEncoder>();
    instance->_renderContext = renderContext;
    
    return instance;
#endif

    return nullptr;
}
