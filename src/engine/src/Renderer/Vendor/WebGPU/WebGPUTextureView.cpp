#include "Renderer/Vendor/WebGPU/WebGPUTextureView.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureResource.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUSwapchain.hpp"

void WebGPUTextureView::CreateView(Format format, const Range &levels, TextureType textureType) {
    const std::shared_ptr<WebGPUTextureResource> textureResource = std::static_pointer_cast<WebGPUTextureResource>(_textureResource);
    if(!textureResource) {
        assert(0 && "WebGPUTextureView::CreateView() failed to acquire texture resource.");
        return;
    }

    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = TranslateFormat(format);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    _view = wgpuTextureCreateView(textureResource->GetWGPUTexture(), &viewDescriptor);

    // _view = wgpuSwapChainGetCurrentTextureView(textureResource->GetWGPUTexture());


    if(_view == nullptr) {
        assert(0 && "WebGPUTextureView::CreateView() failed to create view.");
        return;
    }
}

void WebGPUTextureView::FreeView() {
     if(_view) {
         wgpuTextureViewRelease(_view);
     }
}
