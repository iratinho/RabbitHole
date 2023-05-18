#pragma once

class RenderContext;
class Swapchain;
class RenderTarget;

struct SurfaceCreateParams {
    RenderContext* _renderContext;
    Swapchain* _swapChain;
    std::weak_ptr<RenderTarget> _swapChainRenderTarget;
    std::weak_ptr<RenderTarget> _swapChainRenderTargetDepth;
};

struct SurfacePresentParams
{
    unsigned int _frameIndex;
    // Surface will only be presented after this semaphore
    void* _waitSemaphore;
};

class Surface {
public:
    Surface();
    void AllocateSurface(SurfaceCreateParams& params);
    void Present(const SurfacePresentParams& presentParams) const;
    std::shared_ptr<RenderTarget> GetRenderTarget();
    std::shared_ptr<RenderTarget> GetDepthRenderTarget();

    bool IsInitialized() const;

private:
    std::shared_ptr<RenderTarget> _surfaceRenderTarget;
    std::shared_ptr<RenderTarget> _surfaceRenderTargetDepth;
    Swapchain* _swapChain = nullptr;
    RenderContext* _renderContext = nullptr;

    bool _bIsInitialized = false;
};