#include "Renderer/Swapchain.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Event.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "window.hpp"

Swapchain::Swapchain(Device* device)
    : m_bIsSwapchainDirty(false)
    , m_nextSwapchainImageIndex(0)
    , _device(device)
    , m_swapchain(nullptr)
{
}

bool Swapchain::Initialize() {
    ((VKDevice*)_device)->CreateSwapChain(m_swapchain, m_swapchainImages);
    if(!CreateRenderTargets()) {
        std::cerr << "[Error]: Swapchain failed to create render targets." << std::endl;
        return false;
    }
    
    CreateSyncPrimitives();

    return true;
}

void Swapchain::Recreate() {
    VkFunc::vkDestroySwapchainKHR(((VKDevice*)_device)->GetLogicalDeviceHandle(), m_swapchain, nullptr);
    Initialize();
}

bool Swapchain::RequestNewPresentableImage(uint32_t index) {
    std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(_events.peekAdvanced());
    
    if(!vkEvent) {
        return false;;
    }
    
    const bool bIsOutDated = m_bIsSwapchainDirty || !((VKDevice*)_device)->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, vkEvent->GetVkSemaphore());

    if (bIsOutDated /* Dirty because of a possible window resize */) {
        // Keep pooling events until the size of the window is no longer invalid
        while (((VKDevice*)_device)->GetSwapchainExtent().y == 0 || ((VKDevice*)_device)->GetSwapchainExtent().x == 0) {
            ((VKDevice*)_device)->GetWindow()->PoolEvents();
        }
        

        VkFunc::vkDeviceWaitIdle(((VKDevice*)_device)->GetLogicalDeviceHandle());

        Recreate();
//        m_renderContext->GetRenderSystem()->ReleaseResources();
        return false;
    }

    return true;
}

unsigned int Swapchain::RequestNewPresentableImage() {
    
    std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(_events.peekAdvanced());
    
    if(!vkEvent) {
        return false;
    }
    
    ((VKDevice*)_device)->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, vkEvent->GetVkSemaphore());
    m_ColorTextures[m_nextSwapchainImageIndex]->SetTextureLayout(ImageLayout::LAYOUT_UNDEFINED);
    return m_nextSwapchainImageIndex;
}

void Swapchain::MarkSwapchainDirty() {
    m_bIsSwapchainDirty = true;    
}

std::shared_ptr<Texture2D> Swapchain::GetSwapchainTexture(ESwapchainTextureType type, uint32_t index)
{
    if(type == ESwapchainTextureType::COLOR)
    {
        return m_ColorTextures[index];
    }

    if(type == ESwapchainTextureType::DEPTH)
    {
        return m_DepthTextures[index];
    }

    return nullptr;
}

bool Swapchain::CreateRenderTargets()
{
    m_ColorTextures.clear();
    m_DepthTextures.clear();
    
    m_ColorTextures.resize(GetSwapchainImageCount());
    m_DepthTextures.resize(GetSwapchainImageCount());
    
    // Create the swapchain render targets and cache them in the render graph
    for (int i = 0; i < GetSwapchainImageCount(); ++i)
    {
        const unsigned int width = _device->GetSwapchainExtent().x;
        const unsigned int height = _device->GetSwapchainExtent().y;
        auto colorTexture = Texture2D::MakeFromExternalResource(width, height, Format::FORMAT_B8G8R8A8_SRGB, TextureFlags::Tex_COLOR_ATTACHMENT);
        if(!colorTexture->Initialize(_device)) {
            return false;
        }
        
        colorTexture->CreateResource(m_swapchainImages[i]);
                
        auto sceneDepthTexture = Texture2D::MakeTexturePass(width, height, Format::FORMAT_D32_SFLOAT, TextureFlags::Tex_DEPTH_ATTACHMENT);
        if(!sceneDepthTexture->Initialize(_device)) {
            return false;
        }
        
        sceneDepthTexture->CreateResource();
        
        m_ColorTextures[i] = colorTexture;
        m_DepthTextures[i] = sceneDepthTexture;
    }

    return true;

}

bool Swapchain::CreateSyncPrimitives() {
    for (int i = 0; i < GetSwapchainImageCount(); ++i) {
        
        std::shared_ptr<Event> event = Event::MakeEvent({_device});
        _events.push(event);
    }
    
    return true;
}

