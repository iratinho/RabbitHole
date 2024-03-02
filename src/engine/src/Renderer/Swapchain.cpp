#include "Renderer/Swapchain.hpp"
//#include "Renderer/RenderSystem.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Texture2D.hpp"
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
    const bool bIsOutDated = m_bIsSwapchainDirty || !m_renderContext->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, _semaphores.peekAdvanced());
        
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
    m_renderContext->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, _semaphores.peekAdvanced());
    m_colorRenderTargets[m_nextSwapchainImageIndex]->GetTexture()->SetTextureLayout(ImageLayout::LAYOUT_PRESENT);
    return m_nextSwapchainImageIndex;
}

void Swapchain::MarkSwapchainDirty() {
    m_bIsSwapchainDirty = true;    
}

std::shared_ptr<RenderTarget> Swapchain::GetSwapchainRenderTarget(ESwapchainRenderTargetType type, uint32_t index)
{
    if(type == ESwapchainRenderTargetType::COLOR)
    {
        return m_colorRenderTargets[index];
    }

    if(type == ESwapchainRenderTargetType::DEPTH)
    {
        return m_depthRenderTargets[index];
    }

    return nullptr;
}

bool Swapchain::CreateRenderTargets()
{
    m_colorRenderTargets.clear();
    m_depthRenderTargets.clear();
    
    m_colorRenderTargets.resize(GetSwapchainImageCount());
    m_depthRenderTargets.resize(GetSwapchainImageCount());
    
    // Create the swapchain render targets and cache them in the render graph
    for (int i = 0; i < GetSwapchainImageCount(); ++i)
    {
        RenderTargetParams colorRenderTargetParams;
        colorRenderTargetParams._textureParams.format = Format::FORMAT_B8G8R8A8_SRGB;
        colorRenderTargetParams._textureParams.flags = static_cast<TextureFlags>(Tex_COLOR_ATTACHMENT | Tex_PRESENTATION);
        colorRenderTargetParams._textureParams._height = m_renderContext->GetSwapchainExtent().height;
        colorRenderTargetParams._textureParams._width = m_renderContext->GetSwapchainExtent().width;
        colorRenderTargetParams._textureParams._sampleCount = 0;
        colorRenderTargetParams._textureParams._hasSwapchainUsage = true;
        colorRenderTargetParams._usageFlags = Rt_Swapchain;
        
        // Texture color_texture = Texture(m_renderContext, color_texture_params, m_swapchainImages[i]);
        const auto colorRenderTarget = std::make_shared<RenderTarget>(m_renderContext, colorRenderTargetParams);

        if(!colorRenderTarget->Initialize()) {
            return false;
        }
        
        colorRenderTarget->SetTextureResource(m_swapchainImages[i]);
        
        RenderTargetParams depthRenderTargetParams;
        depthRenderTargetParams._textureParams.format = Format::FORMAT_D32_SFLOAT;
        depthRenderTargetParams._textureParams.flags = static_cast<TextureFlags>(Tex_DEPTH_ATTACHMENT | Tex_PRESENTATION);
        depthRenderTargetParams._textureParams._sampleCount = 0;
        depthRenderTargetParams._textureParams._width = m_renderContext->GetSwapchainExtent().width;
        depthRenderTargetParams._textureParams._height = m_renderContext->GetSwapchainExtent().height;
        depthRenderTargetParams._textureParams._hasSwapchainUsage = true;
        depthRenderTargetParams._usageFlags = Rt_None;

        const auto scene_depth_render_target = std::make_shared<RenderTarget>(m_renderContext, depthRenderTargetParams);
        if(!scene_depth_render_target->Initialize()) {
            return false;
        }

        m_colorRenderTargets[i] = colorRenderTarget;
        m_depthRenderTargets[i] = scene_depth_render_target;
    }

    return true;

}

bool Swapchain::CreateSyncPrimitives() {
    for (int i = 0; i < GetSwapchainImageCount(); ++i) {
        // Semaphore that will be signaled when the swapchain has an image ready
        VkSemaphoreCreateInfo acquire_semaphore_create_info;
        acquire_semaphore_create_info.flags = 0;
        acquire_semaphore_create_info.pNext = nullptr;
        acquire_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkSemaphore semaphore;
        const VkResult result = VkFunc::vkCreateSemaphore(m_renderContext->GetLogicalDeviceHandle(), &acquire_semaphore_create_info, nullptr, &semaphore);

        if (result != VK_SUCCESS) {
            return false;
        }
        
        _semaphores.push(semaphore);
    }
    
    return true;
}

