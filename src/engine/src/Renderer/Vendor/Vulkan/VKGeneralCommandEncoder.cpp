#include "Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/GPUDefinitions.h"
#include "Renderer/Texture2D.hpp"

// Consider passing the layout inside resource instead of texture2D TODO: Add support for multiple textures barriers
void VKGeneralCommandEncoder::MakeImageBarrier(Texture2D* texture2D, ImageLayout after) {
    VkImageLayout oldLayout = TranslateImageLayout(texture2D->GetCurrentLayout());
    VkImageLayout newLayout = TranslateImageLayout(after);
    
    if(oldLayout == newLayout) {
        return;
    }
    
    bool bIsDepth = after == ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;
    
    VkImageSubresourceRange subresource;
    subresource.aspectMask = bIsDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseMipLevel = 0;
    subresource.levelCount = 1;
    subresource.baseArrayLayer = 0;
    subresource.layerCount = 1;
    
    VkAccessFlags srcAccessMask, dstAccessMask;
    std::tie(srcAccessMask, dstAccessMask) = GetAccessFlagsFromLayout(oldLayout, newLayout);
    
    VkTextureResource* textureResource = (VkTextureResource*)texture2D->GetResource().get();
    
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = VK_NULL_HANDLE;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 0;
    barrier.image = textureResource->GetImage();
    barrier.subresourceRange = subresource;
    
    VkPipelineStageFlags srcStage, dstStage;
    std::tie(srcStage, dstStage) = GetPipelineStageFlagsFromLayout(oldLayout, newLayout);
    
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
    
    texture2D->SetTextureLayout(after);
}

