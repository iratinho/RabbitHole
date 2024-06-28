#include "Renderer/Vendor/Vulkan/VKFence.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/render_context.hpp"

void VKFence::Initialize() {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.pNext = nullptr;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence = VK_NULL_HANDLE;
    if(VkFunc::vkCreateFence(_params._renderContext->GetLogicalDeviceHandle(), &fenceInfo, nullptr, &_fence) != VK_SUCCESS) {
        assert(0 && "Unable to create vulkan fence");
        return;
    }
}

void VKFence::Wait() {
    VkFunc::vkWaitForFences(_params._renderContext->GetLogicalDeviceHandle(), 1, &_fence, VK_TRUE, UINT64_MAX);
    VkFunc::vkResetFences(_params._renderContext->GetLogicalDeviceHandle(), 1, &_fence); // We assume that we reset all the time
}
