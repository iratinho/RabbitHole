#include "Renderer/Fence.h"
#include "Renderer/render_context.h"

Fence::Fence(RenderContext* render_context)
    : render_context_(render_context)
{
}

Fence::Fence(RenderContext* render_context, VkFence fence)
    : fence_(fence)
    , render_context_(render_context)
{
}

bool Fence::Initialize()
{
    VkFenceCreateInfo fence_create_info;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fence_create_info.pNext = nullptr;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    return render_context_->MakeFence(fence_create_info, &fence_);
}

void Fence::ResetFence() {
    if(render_context_) {
        render_context_->ResetFence(fence_);    
    }
}

void Fence::WaitFence() {
    if(render_context_) {
        render_context_->WaitFence(fence_);
    }
}

VkFence Fence::GetResource() {
    return fence_;
}