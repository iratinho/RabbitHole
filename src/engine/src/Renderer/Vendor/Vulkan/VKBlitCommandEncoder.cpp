#include "Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/render_context.hpp"

void VKBlitCommandEncoder::UploadBuffer(std::shared_ptr<Buffer> buffer) {
    VKBuffer* vkBuffer = (VKBuffer*) buffer.get();
    
    if(vkBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = buffer->GetSize();
        
        VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
        VkFunc::vkCmdCopyBuffer(commandBuffer, vkBuffer->GetHostBuffer(), vkBuffer->GetLocalBuffer(), 1, &copyRegion);
    }
}
