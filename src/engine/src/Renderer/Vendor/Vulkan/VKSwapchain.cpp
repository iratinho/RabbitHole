#include "Renderer/Vendor/Vulkan/VKSwapchain.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"

bool VKSwapchain::Initialize() {
    return true;
}

void VKSwapchain::Shutdown() {
    Cleanup();
}

bool VKSwapchain::PrepareNextImage() {
    if(IsDirty()) {
        Recreate();
    }

    std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(_events.peekAdvanced());
    if(!vkEvent) {
        return false;
    }

    const auto device = dynamic_cast<VKDevice *>(_device);
    if(!device) {
        return false;
    }

    const VkResult result = VkFunc::vkAcquireNextImageKHR(device->GetLogicalDeviceHandle(),
        _swapchain,
        0,
        vkEvent->GetVkSemaphore(),
        VK_NULL_HANDLE,
        &_currentIdx);

    if(result != VK_SUCCESS) {
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            Recreate();
            return PrepareNextImage();
        }

        // To handle more cases later
        assert(0);
        return false;
    }

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

void VKSwapchain::Cleanup() {
    const auto device = dynamic_cast<VKDevice *>(_device);
    if(!device) {
        assert(0);
        return;
    }

    // Wait for device to be idle and clean up everything.
    VkFunc::vkDeviceWaitIdle(device->GetLogicalDeviceHandle());

    if(_swapchain != VK_NULL_HANDLE) {
        VkFunc::vkDestroySwapchainKHR(device->GetLogicalDeviceHandle(), _swapchain, nullptr);
    }

    _colorTextures.clear();
    _depthTextures.clear();
}

void VKSwapchain::Recreate() {
    Cleanup();

    dynamic_cast<VKDevice *>(_device)->CreateSwapChain(_swapchain, _images);
    if(!CreateRenderTargets()) {
        std::cerr << "[Error]: Swapchain failed to create render targets." << std::endl;
        return;
    }

    CreateSyncPrimitives();

    _isDirty = false;
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

        auto sceneDepthTexture = Texture2D::MakeAttachmentDepthTexture(width, height);
        if(!sceneDepthTexture->Initialize(_device)) {
            return false;
        }

        sceneDepthTexture->CreateResource(nullptr);

        _colorTextures[i] = colorTexture;
        _depthTextures[i] = sceneDepthTexture;
    }

    return true;
}

bool VKSwapchain::CreateSyncPrimitives() {
    for (int i = 0; i < GetImageCount(); ++i) {
        std::shared_ptr event = std::move(Event::MakeEvent({_device}));
        _events.push(event);
    }

    return true;
}
