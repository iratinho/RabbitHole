#pragma once
#include "RenderPass/RenderPass.h"

class Fence : public IFence {
public:
    Fence(RenderContext* render_context);
    Fence(RenderContext* render_context, VkFence fence);

    virtual bool Initialize() override;
    virtual void ResetFence() override;
    virtual void WaitFence() override;
    virtual VkFence GetResource() override;

private:
    VkFence fence_;
    RenderContext* render_context_;
};