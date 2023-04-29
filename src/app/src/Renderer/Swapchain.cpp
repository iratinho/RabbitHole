#include "Renderer/Swapchain.h"
#include "Renderer/RenderSystem.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/Texture.h"
#include "window.h"

Swapchain::Swapchain(RenderContext* renderContext)
    : ISwapchain()
    , m_bIsSwapchainDirty(false)
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
    const bool bIsOutDated = !m_renderContext->AcquireNextImage(m_swapchain, m_nextSwapchainImageIndex, m_semaphore[index]);
    if (bIsOutDated /* Dirty because of a possible window resize */) {
        // Keep pooling events until the size of the window is no longer invalid
        while (m_renderContext->GetSwapchainExtent().height == 0 || m_renderContext->GetSwapchainExtent().width == 0) {
            m_renderContext->GetWindow()->PoolEvents();
        }

        VkFunc::vkDeviceWaitIdle(m_renderContext->GetLogicalDeviceHandle());

        Recreate();
        m_renderContext->GetRenderSystem()->ReleaseResources();
        return false;
    }

    return true;
}

void Swapchain::MarkSwapchainDirty() {
    m_bIsSwapchainDirty = true;    
}

RenderTarget* Swapchain::GetSwapchainRenderTarget(ISwapchain::ESwapchainRenderTargetType type, uint32_t index)
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
    for (const auto colorRenderTarget : m_colorRenderTargets) {
        delete colorRenderTarget;
    }

    for (const auto depthRenderTarget : m_depthRenderTargets) {
        delete depthRenderTarget;
    }

    m_colorRenderTargets.resize(GetSwapchainImageCount());
    m_depthRenderTargets.resize(GetSwapchainImageCount());
    
    // Create the swapchain render targets and cache them in the render graph
    for (int i = 0; i < GetSwapchainImageCount(); ++i)
    {
        RenderTargetParams colorRenderTargetParams;
        colorRenderTargetParams._usageFlags = Rt_Swapchain;
        colorRenderTargetParams._textureParams.format = VK_FORMAT_B8G8R8A8_SRGB;
        colorRenderTargetParams._textureParams.flags = static_cast<TextureUsageFlags>(Tex_COLOR_ATTACHMENT | Tex_PRESENTATION);
        colorRenderTargetParams._textureParams._height = m_renderContext->GetSwapchainExtent().height;
        colorRenderTargetParams._textureParams._width = m_renderContext->GetSwapchainExtent().width;
        colorRenderTargetParams._textureParams._sampleCount = 0;
        colorRenderTargetParams._textureParams._hasSwapchainUsage = true;
        
        // Texture color_texture = Texture(m_renderContext, color_texture_params, m_swapchainImages[i]);
        const auto colorRenderTarget = new RenderTarget(m_renderContext, colorRenderTargetParams);
        colorRenderTarget->SetTextureResource(m_swapchainImages[i]);

        if(!colorRenderTarget->Initialize()) {
            return false;
        }
        
        TextureParams depthColorParams;
        depthColorParams.format = VK_FORMAT_D32_SFLOAT;
        depthColorParams.flags = static_cast<TextureUsageFlags>(Tex_DEPTH_ATTACHMENT | Tex_PRESENTATION);
        depthColorParams._sampleCount = 0;
        depthColorParams._width = m_renderContext->GetSwapchainExtent().width;
        depthColorParams._height = m_renderContext->GetSwapchainExtent().height;
        depthColorParams._hasSwapchainUsage = true;

        const auto scene_depth_render_target = new RenderTarget(m_renderContext, RenderTargetParams(depthColorParams));
        if(!scene_depth_render_target->Initialize()) {
            return false;
        }

        m_colorRenderTargets[i] = colorRenderTarget;
        m_depthRenderTargets[i] = scene_depth_render_target;
    }

    return true;

}

bool Swapchain::CreateSyncPrimitives() {
    m_semaphore.resize(GetSwapchainImageCount());
    
    for (int i = 0; i < GetSwapchainImageCount(); ++i) {
        // Semaphore that will be signaled when the swapchain has an image ready
        VkSemaphoreCreateInfo acquire_semaphore_create_info;
        acquire_semaphore_create_info.flags = 0;
        acquire_semaphore_create_info.pNext = nullptr;
        acquire_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        const VkResult result = VkFunc::vkCreateSemaphore(m_renderContext->GetLogicalDeviceHandle(), &acquire_semaphore_create_info, nullptr, &m_semaphore[i]);

        if (result != VK_SUCCESS)
        {
            return false;
        }    
    }
    
    return true;
}

