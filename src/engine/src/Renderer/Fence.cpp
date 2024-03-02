#include "Renderer/Fence.hpp"
#include "Renderer/render_context.hpp"

Fence::Fence(RenderContext* render_context)
    : _fence(VK_NULL_HANDLE)
    , _renderContext(render_context)
{
}

Fence::Fence(RenderContext* render_context, void* fence)
    : _fence(fence)
    , _renderContext(render_context)
{
}

bool Fence::AllocateFence() {
    VkFenceCreateInfo fence_create_info;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fence_create_info.pNext = nullptr;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    _fence = _renderContext->AllocateFence(fence_create_info);

    if(!_fence) {
        return false;
    }

    return true;
}

void Fence::ResetFence() {
    if(_renderContext) {
        _renderContext->ResetFence(static_cast<VkFence>(_fence));    
    }
}

void Fence::WaitFence() {
    if(_renderContext) {
        _renderContext->WaitFence(static_cast<VkFence>(_fence));
    }
}

void* Fence::GetResource() {
    return _fence;
}
