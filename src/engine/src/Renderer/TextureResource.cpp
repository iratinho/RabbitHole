#include "Renderer/TextureResource.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#endif

TextureResource::TextureResource(Device* device, Texture2D* texture, bool bIsExternalResource)
    : _device(device)
    , _texture(texture)
    , _bIsExternalResource(bIsExternalResource) {
}

std::unique_ptr<TextureResource> TextureResource::MakeResource(Device* device, Texture2D* texture, bool bIsExternalResource) {
#ifdef USING_VULKAN_API
    return std::make_unique<VkTextureResource>(device, texture, bIsExternalResource);
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
