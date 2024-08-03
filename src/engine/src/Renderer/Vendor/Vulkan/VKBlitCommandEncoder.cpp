#include "Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
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

void VKBlitCommandEncoder::UploadImageBuffer(std::shared_ptr<Texture2D> texture) {
    // TODO: big todo
    
    auto resource = std::static_pointer_cast<VkTextureResource>(texture->GetResource());
    auto buffer = std::static_pointer_cast<VKBuffer>(resource->GetBuffer());
    
    VkImage image = resource ? resource->GetImage() : VK_NULL_HANDLE;
    VkBuffer hostBuffer = buffer ? buffer->GetHostBuffer() : VK_NULL_HANDLE;
        
    if(buffer && image && hostBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = buffer->GetSize();
        
        VkBufferImageCopy imageCopy = {};
        
        VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
        VkFunc::vkCmdCopyBufferToImage(commandBuffer, hostBuffer, image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    }
    
    // TODO: After the copy we also need to ensure that we transition the image to be consumed. Should we do it here?
}
