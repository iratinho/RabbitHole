#include "Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"

#include "Renderer/Texture2D.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"

void VKBlitCommandEncoder::UploadBuffer(std::shared_ptr<Buffer> buffer) {
    VKBuffer* vkBuffer = dynamic_cast<VKBuffer*>(buffer.get());
    
    if(vkBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = buffer->GetSize();
        
        VkCommandBuffer commandBuffer = dynamic_cast<VKCommandBuffer *>(_commandBuffer)->GetVkCommandBuffer();
        VkFunc::vkCmdCopyBuffer(commandBuffer, vkBuffer->GetHostBuffer(), vkBuffer->GetLocalBuffer(), 1, &copyRegion);
    }
}

void VKBlitCommandEncoder::UploadImageBuffer(std::shared_ptr<Texture2D> texture) {
    if(!texture) {
        assert(0);
        return;
    }
    
    auto resource = std::static_pointer_cast<VkTextureResource>(texture->GetResource());
    auto buffer = std::static_pointer_cast<VKBuffer>(resource->GetBuffer());
    
    VkImage image = resource ? resource->GetImage() : VK_NULL_HANDLE;
    VkBuffer hostBuffer = buffer ? buffer->GetHostBuffer() : VK_NULL_HANDLE;
        
    if(buffer && image && hostBuffer) {
        VkBufferCopy copyRegion {};
        copyRegion.size = buffer->GetSize();
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        
        VkImageSubresourceLayers subResource {};
        subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subResource.layerCount = 1;
        subResource.mipLevel = 0;
        subResource.baseArrayLayer = 0;
        
        VkExtent3D extent;
        extent.width = texture->GetWidth();
        extent.height = texture->GetHeight();
        extent.depth = 1;
        
        VkBufferImageCopy imageCopy {};
        imageCopy.bufferImageHeight = 0;
        imageCopy.bufferOffset = 0;
        imageCopy.bufferRowLength = 0;
        imageCopy.imageExtent = extent;
        imageCopy.imageSubresource = subResource;
        imageCopy.imageOffset = {0 , 0, 0};
        
        VkCommandBuffer commandBuffer = dynamic_cast<VKCommandBuffer *>(_commandBuffer)->GetVkCommandBuffer();
        VkFunc::vkCmdCopyBufferToImage(commandBuffer, hostBuffer, image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
        
        // Texture is now on the gpu, lets clear the dirty flag
        buffer->ClearDirty();
    }
}

void VKBlitCommandEncoder::CopyImageToImage(const std::shared_ptr<Texture2D>& src, const std::shared_ptr<Texture2D>& dst) {
    auto srcResource = std::static_pointer_cast<VkTextureResource>(src->GetResource());
    auto dstResource = std::static_pointer_cast<VkTextureResource>(dst->GetResource());
    
    VkImage srcImage = srcResource ? srcResource->GetImage() : nullptr;
    VkImage dstImage = dstImage ? dstResource->GetImage() : nullptr;
    
    if(!srcImage || !dstImage) {
        assert(false && "Invalid src | dst images to do copy to image");
        return;
    }
    
    VkCommandBuffer commandBuffer = dynamic_cast<VKCommandBuffer *>(_commandBuffer)->GetVkCommandBuffer();
    
    VkExtent3D extent;
    extent.width = src->GetWidth();
    extent.height = dst->GetHeight();
    extent.depth = 1;
    
    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = extent;

    VkFunc::vkCmdCopyImage(commandBuffer,
                   srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &copyRegion);
}
