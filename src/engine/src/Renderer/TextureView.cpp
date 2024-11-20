#include "Renderer/TextureView.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKTextureView.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUTextureView.hpp"
#endif

TextureView::TextureView(Device* device, std::shared_ptr<TextureResource> textureResource)
    : _device(device)
    , _textureResource(textureResource) {
}

std::unique_ptr<TextureView> TextureView::MakeTextureView(Device *device, std::shared_ptr<TextureResource> textureResource) {
#ifdef VULKAN_BACKEND
    return std::make_unique<VKTextureView>(device, textureResource);
#else
    return std::make_unique<WebGPUTextureView>(device, textureResource);
#endif
    
    return nullptr;
}
