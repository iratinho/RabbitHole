#include "Renderer/Vendor/WebGPU/WebGPUSwapchain.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUWindow.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
#include "Renderer/Texture2D.hpp"
#include "window.hpp"

bool WebGPUSwapchain::Initialize() {
    return true;
}

void WebGPUSwapchain::Shutdown() {
}

bool WebGPUSwapchain::PrepareNextImage() {
    Recreate();

    return true;
}

std::shared_ptr<Texture2D> WebGPUSwapchain::GetTexture(ESwapchainTextureType_ type) {
    if(type == _COLOR) {
        return _colorTexture;
    }

    if(type == _DEPTH) {
        return _depthTexture;
    }

    return nullptr;
}

std::shared_ptr<Event> WebGPUSwapchain::GetSyncEvent() {
    return nullptr;
}

void WebGPUSwapchain::Recreate() {
    WebGPUWindow* window = _device ? reinterpret_cast<WebGPUWindow*>(_device->GetWindow()) : nullptr;
    if(!window) {
        std::cerr << "WebGPUSwapchain::Recreate() - Failed to acquire window" << std::endl;
        return;
    }

    auto* _device = reinterpret_cast<WebGPUDevice*>(window->GetDevice());
    if(!_device) {
        std::cerr << "WebGPUSwapchain::Recreate() - Failed to acquire device" << std::endl;
        return;
    }
    
    _colorTexture.reset();

    WGPUTextureFormat format = wgpuSurfaceGetPreferredFormat(window->GetWebGPUSurface(), _device->GetWebGPUAdapter());

    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(window->GetWebGPUSurface(), &surfaceTexture);
    
    std::uint32_t surfaceWidth = wgpuTextureGetWidth(surfaceTexture.texture);
    std::uint32_t surfaceHeight = wgpuTextureGetHeight(surfaceTexture.texture);
    
    _colorTexture = Texture2D::MakeFromExternalResource(
        surfaceWidth,
        surfaceHeight,
        //FORMAT_B8G8R8A8_UNORM,
        FORMAT_B8G8R8A8_SRGB,
        TextureFlags::Tex_COLOR_ATTACHMENT);

    if(!_colorTexture->Initialize(_device)) {
        std::cerr << "WebGPUSwapchain::Recreate() - Failed to initialize texture" << std::endl;
        return;
    }

    _colorTexture->CreateResource(surfaceTexture.texture);
    
    _depthTexture = Texture2D::MakeAttachmentDepthTexture(surfaceWidth, surfaceHeight);
    
    if(!_depthTexture->Initialize(_device)) {
        std::cerr << "WebGPUSwapchain::Recreate() - Failed to initialize texture" << std::endl;
        return;
    }
    
    _depthTexture->CreateResource(nullptr);
}
