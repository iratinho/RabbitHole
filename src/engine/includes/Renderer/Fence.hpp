#pragma once

class RenderContext;

class Fence {
public:
    Fence(RenderContext* render_context);
    Fence(RenderContext* render_context, void* fence);

    bool AllocateFence();
    void ResetFence();
    void WaitFence();
    void* GetResource();

private:
    void* _fence;
    RenderContext* _renderContext;
};
