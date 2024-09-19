#pragma once

class Swapchain;
class RenderContext;
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

private:
    std::shared_ptr<RenderTarget> _surfaceRenderTarget;
    std::shared_ptr<RenderTarget> _surfaceRenderTargetDepth;
    Swapchain* _swapChain;
    RenderContext* _renderContext;
};
