#pragma once
#include "RenderPass/RenderPass.h"

class Fence : public IFence {
public:
    Fence(RenderContext* render_context);
    Fence(RenderContext* render_context, void* fence);

    virtual bool AllocateFence() override;
    virtual void ResetFence() override;
    virtual void WaitFence() override;
    virtual void* GetResource() override;

private:
    void* _fence;
    RenderContext* _renderContext;
};