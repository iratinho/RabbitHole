#include "Renderer/Vendor/Vulkan/VKTransferContext.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/render_context.hpp"

void VKTransferContext::EnqueueBufferSync(std::shared_ptr<Buffer> buffer) {
    if(!buffer) {
        return;
    }
    
    if(_isPending == false) {
        InitializeResources();
        
        VkCommandBufferBeginInfo transferCommandInfo {};
        transferCommandInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        transferCommandInfo.pNext = nullptr;
        transferCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkFunc::vkBeginCommandBuffer(_commandBuffer, &transferCommandInfo);
    }
    
    VKBuffer* vkBuffer = (VKBuffer*) buffer.get();
    
    if(vkBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = buffer->GetSize();

        VkFunc::vkCmdCopyBuffer(_commandBuffer, vkBuffer->GetHostBuffer(), vkBuffer->GetLocalBuffer(), 1, &copyRegion);
    }
    
    _bufferList.push_back(buffer);
    _isPending = true;
}

void VKTransferContext::Flush() {
    if(_isPending == false || !_commandPool || !_commandBuffer || !_fence) {
        return;
    }
        
    VkFunc::vkEndCommandBuffer(_commandBuffer);
    
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    
    VkFunc::vkQueueSubmit(_renderContext->GetGraphicsQueueHandle(), 1, &submitInfo, _fence);
    VkFunc::vkWaitForFences(_renderContext->GetLogicalDeviceHandle(), 1, &_fence, VK_TRUE, UINT64_MAX);
    VkFunc::vkResetCommandPool(_renderContext->GetLogicalDeviceHandle(), _commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    
    _bufferList.clear();
    _isPending = false;
}

void VKTransferContext::InitializeResources() {
    if(!_commandPool) {
        VkCommandPoolCreateInfo commandPoolInfo {};
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.pNext = nullptr;
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = _renderContext->GetGraphicsQueueIndex();

        VkFunc::vkCreateCommandPool(_renderContext->GetLogicalDeviceHandle(), &commandPoolInfo, nullptr, &_commandPool);
    }
            
    if(!_commandBuffer) {
        VkCommandBufferAllocateInfo commandBuffer {};
        commandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBuffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBuffer.commandPool = _commandPool; // We should create the pool here
        commandBuffer.commandBufferCount = 1;
        
        VkFunc::vkAllocateCommandBuffers(_renderContext->GetLogicalDeviceHandle(), &commandBuffer, &_commandBuffer);
    }
    
    if(!_fence) {
        VkFenceCreateInfo fenceInfo {};
        fenceInfo.flags = 0;
        fenceInfo.pNext = nullptr;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        
        // TODO destroy fence
        VkFunc::vkCreateFence(_renderContext->GetLogicalDeviceHandle(), &fenceInfo, nullptr, &_fence);
    }
}
