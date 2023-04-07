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

void Swapchain::Initialize() {
    m_renderContext->CreateSwapChain(m_swapchain, m_swapchainImages);
    CreateRenderTargets();
    CreateSyncPrimitives();
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
        TextureParams color_texture_params;
        color_texture_params.format = VK_FORMAT_B8G8R8A8_SRGB;
        color_texture_params.height = m_renderContext->GetSwapchainExtent().height;
        color_texture_params.width = m_renderContext->GetSwapchainExtent().width;
        color_texture_params.sample_count = 0;
        color_texture_params.has_swapchain_usage = true;
        
        Texture color_texture = Texture(m_renderContext, color_texture_params, m_swapchainImages[i]);
        const auto color_depth_render_target = new RenderTarget(std::move(color_texture));

        if(!color_depth_render_target->Initialize())
        {
            return false;
        }
        
        TextureParams depth_color_params;
        depth_color_params.format = VK_FORMAT_D32_SFLOAT;
        depth_color_params.sample_count = 0;
        depth_color_params.width = m_renderContext->GetSwapchainExtent().width;
        depth_color_params.height = m_renderContext->GetSwapchainExtent().height;
        depth_color_params.has_swapchain_usage = true;

        const auto scene_depth_render_target = new RenderTarget(m_renderContext, depth_color_params);

        if(!scene_depth_render_target->Initialize())
        {
            return false;
        }

        m_colorRenderTargets[i] = color_depth_render_target;
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

