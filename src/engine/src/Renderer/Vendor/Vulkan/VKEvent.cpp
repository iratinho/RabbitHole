#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"

void VKEvent::Initialize() {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.flags = 0;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = VkFunc::vkCreateSemaphore(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), &semaphoreInfo, nullptr, &_semaphore);
    if(result != VK_SUCCESS) {
        assert(0 && "Unable to create vulkan semaphore");
        return;
    }
}
