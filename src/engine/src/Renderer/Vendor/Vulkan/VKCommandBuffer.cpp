#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#include "Renderer/Vendor/Vulkan/VKFence.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/Swapchain.hpp"

// For now it makes sense to be a static value since we wont have other threads
VkCommandPool VKCommandBuffer::_commandPool = VK_NULL_HANDLE;

bool VKCommandBuffer::Initialize() {
    if(!_commandPool) {
        VkCommandPoolCreateInfo commandPoolInfo {};
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.pNext = nullptr;
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = _params._renderContext->GetGraphicsQueueIndex();

        if (VkFunc::vkCreateCommandPool(_params._renderContext->GetLogicalDeviceHandle(), &commandPoolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
            return false;
        }
    }
            
    if(!_commandBuffer) {
        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandPool = static_cast<VkCommandPool>(_commandPool);
        commandBufferInfo.pNext = nullptr;
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandBufferCount = 1;

        if (VkFunc::vkAllocateCommandBuffers(_params._renderContext->GetLogicalDeviceHandle(), &commandBufferInfo, &_commandBuffer) != VK_SUCCESS) {
            return false;
        }
    }
    
    return true;
}

void VKCommandBuffer::BeginRecording() {
    VkFunc::vkResetCommandBuffer(_commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    
    // Temporary
//    VkFunc::vkResetCommandPool(_params._renderContext->GetLogicalDeviceHandle(), _commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = 0;
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;

    if(VkFunc::vkBeginCommandBuffer(static_cast<VkCommandBuffer>(_commandBuffer), &beginInfo) != VK_SUCCESS) {
        assert(0 && "Unable to begin vulkan command buffer recording");
        return;
    }
}

void VKCommandBuffer::EndRecording() {
    if(VkFunc::vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
        assert(0 && "Unable to end vulkan command buffer recording");
        return;
    }
}

void VKCommandBuffer::Submit(std::shared_ptr<Fence> fence) {
    VkFence vkRawFence = VK_NULL_HANDLE;
    std::shared_ptr<VKFence> vkFence = std::static_pointer_cast<VKFence>(fence);
    if(vkFence) {
        vkRawFence = vkFence->GetVkFence();
    }

    // Collect semaphroes that this command buffer must wait before sumit its work
    std::vector<VkSemaphore> waitSemaphores;
    for(auto waitEvent : _waitEvents) {
        std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(waitEvent);
        if(vkEvent) {
            waitSemaphores.push_back(vkEvent->GetVkSemaphore());
        }
    };
    
    // Collect semaphroes that this command buffer must sginal after submiting its work
    std::vector<VkSemaphore> signalSemaphores;
    for(auto signalSemaphore : _signalEvents) {
        std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(signalSemaphore);
        if(vkEvent) {
            signalSemaphores.push_back(vkEvent->GetVkSemaphore());
        }
    };

    VkSubmitInfo submitInfo{};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // How to deal with this wait stage in a generic way?
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.waitSemaphoreCount = (uint32_t) waitSemaphores.size();
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.signalSemaphoreCount = (uint32_t) signalSemaphores.size();
    VkFunc::vkQueueSubmit(_params._renderContext->GetGraphicsQueueHandle(), 1, &submitInfo, vkRawFence);
    
    CommandBuffer::Submit();
}
