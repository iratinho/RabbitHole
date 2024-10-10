#include "Renderer/Vendor/Vulkan/VKFence.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VulkanLoader.hpp"

void VKFence::Initialize() {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.pNext = nullptr;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence = VK_NULL_HANDLE;
    if(VkFunc::vkCreateFence(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), &fenceInfo, nullptr, &_fence) != VK_SUCCESS) {
        assert(0 && "Unable to create vulkan fence");
        return;
    }
}

void VKFence::Wait() {
    VkFunc::vkWaitForFences(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), 1, &_fence, VK_TRUE, UINT64_MAX);
    VkFunc::vkResetFences(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), 1, &_fence); // We assume that we reset all the time
}
