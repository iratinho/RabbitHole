#include "Renderer/TextureResource.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#endif

TextureResource::TextureResource(RenderContext* renderContext, Texture2D* texture, bool bIsExternalResource)
    : _renderContext(renderContext)
    , _texture(texture)
    , _bIsExternalResource(bIsExternalResource) {
}

std::unique_ptr<TextureResource> TextureResource::MakeResource(RenderContext* renderContext, Texture2D* texture, bool bIsExternalResource) {
#ifdef USING_VULKAN_API
    return std::make_unique<VkTextureResource>(renderContext, texture, bIsExternalResource);
#endif
    
    return nullptr;
}

bool TextureResource::IsDirty() {
    if(!_buffer) {
        assert(0);
        return false;
    }
    
    return _buffer->IsDirty();
}
