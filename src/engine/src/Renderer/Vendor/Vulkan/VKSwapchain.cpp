#include "Renderer/Vendor/Vulkan/VKSwapchain.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"

bool VKSwapchain::Initialize() {
    dynamic_cast<VKDevice *>(_device)->CreateSwapChain(_swapchain, _images);
    if(!CreateRenderTargets()) {
        std::cerr << "[Error]: Swapchain failed to create render targets." << std::endl;
        return false;
    }

    CreateSyncPrimitives();

    return true;
}

void VKSwapchain::Shutdown() {
}

bool VKSwapchain::PrepareNextImage() {
    std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(_events.peekAdvanced());

    if(!vkEvent) {
        return false;
    }

    dynamic_cast<VKDevice *>(_device)->AcquireNextImage(_swapchain, _currentIdx, vkEvent->GetVkSemaphore());
    _colorTextures[_currentIdx]->SetTextureLayout(ImageLayout::LAYOUT_UNDEFINED);

    return true;
}

std::shared_ptr<Texture2D> VKSwapchain::GetTexture(ESwapchainTextureType_ type) {
    if(type == _COLOR) {
        return _colorTextures[_currentIdx];
    }

    if(type == _DEPTH) {
        return _depthTextures[_currentIdx];
    }

    return nullptr;
}

std::shared_ptr<Event> VKSwapchain::GetSyncEvent() {
    return _events.getCurrent();
}

bool VKSwapchain::CreateRenderTargets() {
    _colorTextures.clear();
    _depthTextures.clear();

    _colorTextures.resize(GetImageCount());
    _depthTextures.resize(GetImageCount());

    // Create the swapchain render targets and cache them in the render graph
    for (int i = 0; i < GetImageCount(); ++i)
    {
        const unsigned int width = _device->GetSwapchainExtent().x;
        const unsigned int height = _device->GetSwapchainExtent().y;
        auto colorTexture = Texture2D::MakeFromExternalResource(width, height, Format::FORMAT_B8G8R8A8_SRGB, TextureFlags::Tex_COLOR_ATTACHMENT);
        if(!colorTexture->Initialize(_device)) {
            return false;
        }

        colorTexture->CreateResource(_images[i]);

        auto sceneDepthTexture = Texture2D::MakeTexturePass(width, height, Format::FORMAT_D32_SFLOAT, TextureFlags::Tex_DEPTH_ATTACHMENT);
        if(!sceneDepthTexture->Initialize(_device)) {
            return false;
        }

        sceneDepthTexture->CreateResource();

        _colorTextures[i] = colorTexture;
        _depthTextures[i] = sceneDepthTexture;
    }

    return true;
}

bool VKSwapchain::CreateSyncPrimitives() {
    for (int i = 0; i < GetImageCount(); ++i) {
        std::shared_ptr event = Event::MakeEvent({_device});
        _events.push(event);
    }

    return true;
}
