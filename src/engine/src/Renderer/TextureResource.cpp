#include "Renderer/TextureResource.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUTextureResource.hpp"
#endif

TextureResource::TextureResource(Device* device, Texture2D* texture, bool bIsExternalResource)
    : _device(device)
    , _texture(texture)
    , _bIsExternalResource(bIsExternalResource) {
}

std::unique_ptr<TextureResource> TextureResource::MakeResource(Device* device, Texture2D* texture, bool bIsExternalResource) {
#ifdef VULKAN_BACKEND
    return std::make_unique<VkTextureResource>(device, texture, bIsExternalResource);
#else
    return std::make_unique<WebGPUTextureResource>(device, texture, bIsExternalResource);
#endif
    
    return nullptr;
}

void TextureResource::MakeDirty() {
    if(_buffer) {
        _buffer->MarkDirty();
    }
}

void TextureResource::ClearDirty() {
    if(_buffer) {
        _buffer->ClearDirty();
    }
}

bool TextureResource::IsDirty() {
    if(!_buffer) {
        assert(0);
        return false;
    }
    
    return _buffer->IsDirty();
}
