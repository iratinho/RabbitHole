#include "Renderer/GraphicsContext.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/Device.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUGraphicsContext.hpp"
#endif

// TODO create the ideia of graphics, copy and compute queues

GraphicsContext::GraphicsContext() {
}

GraphicsContext::~GraphicsContext() {}

std::unique_ptr<GraphicsContext> GraphicsContext::Create(Device* device) {
#ifdef VULKAN_BACKEND
    auto instance = std::make_unique<VKGraphicsContext>(device);
//    instance->_commandEncoder = CommandEncoder::MakeCommandEncoder(renderContext);
    
    return instance;
#else
    auto instance = std::make_unique<WebGPUGraphicsContext>(device);
    return instance;
#endif
    
    return nullptr;
}

bool GraphicsContext::Initialize() {    
    // TODO have a better way to request desired surface texture
    _gbufferTextures._colorTexture = Texture2D::MakeAttachmentTexture(_device->GetSwapchainExtent().x, _device->GetSwapchainExtent().y, Format::FORMAT_B8G8R8A8_SRGB);
    _gbufferTextures._depthTexture = Texture2D::MakeAttachmentDepthTexture(_device->GetSwapchainExtent().x, _device->GetSwapchainExtent().y);
    
    bool bColorInitialized = _gbufferTextures._colorTexture->Initialize(_device);
    bool bDepthIntialized = _gbufferTextures._depthTexture->Initialize(_device);
    
    if(!bColorInitialized || !bDepthIntialized) {
        assert(false);
        return false;
    }

    _gbufferTextures._colorTexture->CreateResource(nullptr);
    _gbufferTextures._depthTexture->CreateResource(nullptr);
    
    return true;
}
