#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"
#include "Renderer/render_context.hpp"

class RenderTarget;

class Swapchain : public ISwapchain {
public:
    Swapchain(RenderContext* renderContext);

    bool Initialize() override;
    void Recreate() override;
    bool RequestNewPresentableImage(uint32_t index) override;
    void MarkSwapchainDirty() override;

    bool IsSwapchainDirty() const override { return m_bIsSwapchainDirty; };
    uint32_t GetNextPresentableImage() const override { return m_nextSwapchainImageIndex; };

    void* GetNativeHandle() const override { return (void*)m_swapchain; }
    
    // Not in the interface
    std::shared_ptr<RenderTarget> GetSwapchainRenderTarget(ISwapchain::ESwapchainRenderTargetType type, uint32_t index);
    VkSemaphore GetSyncPrimtiive(uint32_t index) { return m_semaphore[index]; };
    int GetSwapchainImageCount() { return 2; }// Hardcoded for now
    
private:
    bool CreateRenderTargets();
    bool CreateSyncPrimitives();
    
    bool m_bIsSwapchainDirty;
    uint32_t m_nextSwapchainImageIndex = 0;
    RenderContext* m_renderContext;

    VkSwapchainKHR m_swapchain;
    std::vector<VkSemaphore> m_semaphore;
    std::vector<VkImage> m_swapchainImages;

    std::vector<std::shared_ptr<RenderTarget>> m_colorRenderTargets;
    std::vector<std::shared_ptr<RenderTarget>> m_depthRenderTargets;
};
