#include "Renderer/TransferContext.hpp"
#include "Renderer/render_context.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKTransferContext.hpp"
#endif

std::unique_ptr<TransferContext> TransferContext::Create(RenderContext *renderContext) {
#ifdef USING_VULKAN_API
    auto transferContext = std::make_unique<VKTransferContext>();
    transferContext->_renderContext = renderContext;
    
    return transferContext;
#endif
    
    return nullptr;
}
