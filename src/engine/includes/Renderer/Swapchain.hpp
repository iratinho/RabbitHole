#pragma once
#include "Renderer/render_context.hpp"
#include "Core/Utils.hpp"

class RenderContext;
class RenderTarget;
class Event;

enum ESwapchainTextureType {
    COLOR,
    DEPTH
};

class Swapchain {
public:
    Swapchain(std::shared_ptr<RenderContext> renderContext);

    bool Initialize();
    void Recreate();
    bool RequestNewPresentableImage(uint32_t index);
    unsigned int RequestNewPresentableImage();

    void MarkSwapchainDirty();

    bool IsSwapchainDirty() const { return m_bIsSwapchainDirty; };
    uint32_t GetNextPresentableImage() const { return m_nextSwapchainImageIndex; };

    void* GetNativeHandle() const { return (void*)m_swapchain; }
    
    // Not in the interface
    std::shared_ptr<Texture2D> GetSwapchainTexture(ESwapchainTextureType type, uint32_t index);
//    VkSemaphore GetSyncPrimtiive(uint32_t index) { return _semaphores.getCurrent(); };
    std::shared_ptr<Event> GetSyncPrimtiive(uint32_t index) { return _events.getCurrent(); };

    int GetSwapchainImageCount() { return 2; }// Hardcoded for now
        
private:
    bool CreateRenderTargets();
    bool CreateSyncPrimitives();
            
    bool m_bIsSwapchainDirty;
    uint32_t m_nextSwapchainImageIndex = 0;
    std::shared_ptr<RenderContext> m_renderContext;

    VkSwapchainKHR m_swapchain;
    CircularBuffer<VkSemaphore,2> _semaphores;
    CircularBuffer<std::shared_ptr<Event>, 2> _events;
    
    std::vector<VkImage> m_swapchainImages;

    std::vector<std::shared_ptr<Texture2D>> m_ColorTextures;
    std::vector<std::shared_ptr<Texture2D>> m_DepthTextures;
};
