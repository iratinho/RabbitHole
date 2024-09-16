#include "Renderer/TextureView.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VkTextureView.hpp"
#endif

TextureView::TextureView(Device* device, std::shared_ptr<TextureResource> textureResource)
    : _device(device)
    , _textureResource(textureResource) {
}

std::unique_ptr<TextureView> TextureView::MakeTextureView(Device *device, std::shared_ptr<TextureResource> textureResource) {
#ifdef USING_VULKAN_API
    return std::make_unique<VkTextureView>(device, textureResource);
#endif
    
    return nullptr;
}
