#include "Renderer/Swapchain.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Event.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#include "window.hpp"

Swapchain::Swapchain(std::shared_ptr<RenderContext> renderContext)
    : m_bIsSwapchainDirty(false)
    , m_nextSwapchainImageIndex(0)
    , m_renderContext(renderContext)
    , m_swapchain(nullptr)
{
}

bool Swapchain::Initialize() {
    m_renderContext->CreateSwapChain(m_swapchain, m_swapchainImages);
    if(!CreateRenderTargets()) {
        std::cerr << "[Error]: Swapchain failed to create render targets." << std::endl;
        return false;
    }
    
    CreateSyncPrimitives();

    return true;
}

void Swapchain::Recreate() {
    VkFunc::vkDestroySwapchainKHR(m_renderContext->GetLogicalDeviceHandle(), m_swapchain, nullptr); // TODO move to context
    Initialize();
}

bool Swapchain::RequestNewPresentableImage(uint32_t index) {
    std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(_events.peekAdvanced());
    
    if(!vkEvent) {
        return false;;
    }
    
    const bool bIsOutDated = m_bIsSwapchainDirty || !m_renderContext->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, vkEvent->GetVkSemaphore());
        
    if (bIsOutDated /* Dirty because of a possible window resize */) {
        // Keep pooling events until the size of the window is no longer invalid
        while (m_renderContext->GetSwapchainExtent().height == 0 || m_renderContext->GetSwapchainExtent().width == 0) {
            m_renderContext->GetWindow()->PoolEvents();
        }

        VkFunc::vkDeviceWaitIdle(m_renderContext->GetLogicalDeviceHandle());

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
    
    m_renderContext->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, vkEvent->GetVkSemaphore());
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
        const unsigned int width = m_renderContext->GetSwapchainExtent().width;
        const unsigned int height = m_renderContext->GetSwapchainExtent().height;
        auto colorTexture = Texture2D::MakeFromExternalResource(width, height, Format::FORMAT_B8G8R8A8_SRGB, TextureFlags::Tex_COLOR_ATTACHMENT);
        if(!colorTexture->Initialize(m_renderContext.get())) {
            return false;
        }
        
        colorTexture->CreateResource(m_swapchainImages[i]);
                
        auto sceneDepthTexture = Texture2D::MakeTexturePass(width, height, Format::FORMAT_D32_SFLOAT, TextureFlags::Tex_DEPTH_ATTACHMENT);
        if(!sceneDepthTexture->Initialize(m_renderContext.get())) {
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
        
        std::shared_ptr<Event> event = Event::MakeEvent({m_renderContext});
        _events.push(event);
        
//        // Semaphore that will be signaled when the swapchain has an image ready
//        VkSemaphoreCreateInfo acquire_semaphore_create_info;
//        acquire_semaphore_create_info.flags = 0;
//        acquire_semaphore_create_info.pNext = nullptr;
//        acquire_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//        
//        VkSemaphore semaphore;
//        const VkResult result = VkFunc::vkCreateSemaphore(m_renderContext->GetLogicalDeviceHandle(), &acquire_semaphore_create_info, nullptr, &semaphore);
//
//        if (result != VK_SUCCESS) {
//            return false;
//        }
        
//        _semaphores.push(semaphore);
    }
    
    return true;
}

