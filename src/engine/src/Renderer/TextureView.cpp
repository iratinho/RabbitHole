#include "Renderer/TextureView.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VkTextureView.hpp"
#endif

TextureView::TextureView(RenderContext* renderContext, std::shared_ptr<TextureResource> textureResource)
    : _renderContext(renderContext)
    , _textureResource(textureResource) {
}

std::unique_ptr<TextureView> TextureView::MakeTextureView(RenderContext *renderContext, std::shared_ptr<TextureResource> textureResource) {
#ifdef USING_VULKAN_API
    return std::make_unique<VkTextureView>(renderContext, textureResource);
#endif
    
    return nullptr;
}
