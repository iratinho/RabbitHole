#include "Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKSamplerManager.hpp"
#include "Renderer/Vendor/Vulkan/VKTextureView.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/GPUDefinitions.h"
#include "Renderer/Texture2D.hpp"


#include "Core/GenericFactory.hpp"

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

// TODO We are always assuming that we only have 1 descriptor set, we need to give support for multiple descriptor sets
// we should have a ShaderInputResourceSet that contains multiple ShaderInputResources
void VKGeneralCommandEncoder::BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) {
    std::size_t hash = hash_value(resources);
    
    VKGraphicsContext* context = GetVKContext();
    if(!context) {
        assert(0);
        return;
    }
    
    VKDescriptorManager* descriptorManager = context->GetDescriptorManager();
    if(!descriptorManager) {
        assert(0);
        return;
    }
    
    VKSamplerManager* samplerManager = context->GetSamplerManager();
    if(!samplerManager) {
        assert(0);
        return;
    }
    
    VkDescriptorSet descriptorSet = descriptorManager->AcquireDescriptorSet(_graphicsContext, resources);
    
    if(descriptorSet == VK_NULL_HANDLE) {
        assert(0);
        return;
    }
        
    std::vector<VkWriteDescriptorSet> writes;
    
    for (auto& resource : resources) {
        VkTextureResource* vkResource = resource._textureResource._texture ? (VkTextureResource*)resource._textureResource._texture->GetResource().get() : nullptr;
        if(!vkResource) {
            assert(0);
            continue;
        }
        
        VkWriteDescriptorSet writeDescriptor {};
        writeDescriptor.dstSet = descriptorSet;
        writeDescriptor.dstBinding = resource._binding._id;
        writeDescriptor.descriptorCount = 1.;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        
        if(resource._binding._type == ShaderInputType::TEXTURE) {
            
            VkSampler sampler = samplerManager->AcquireSampler(_graphicsContext, resource._textureResource._sampler);
            if(sampler == VK_NULL_HANDLE) {
                assert(0);
                continue;
            }
            
            VkTextureView* textureView = (VkTextureView*)resource._textureResource._texture->MakeTextureView();
            if(!textureView) {
                assert(0);
                continue;
            }
            
            VkDescriptorImageInfo imageInfo {};
            imageInfo.imageView = textureView->GetImageView();
            imageInfo.imageLayout = TranslateImageLayout(resource._textureResource._texture->GetCurrentLayout());
            imageInfo.sampler = sampler;
            
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.pImageInfo = &imageInfo;
        }
        
        writes.push_back(writeDescriptor);
    }
    
    VkFunc::vkUpdateDescriptorSets(_graphicsContext->GetDevice()->GetLogicalDeviceHandle(), writes.size(), writes.data(), 0, VK_NULL_HANDLE);
    
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkPipelineLayout layout = ((VKGraphicsPipeline*)shader->GetPipeline())->GetVKPipelineLayout();
    VkFunc::vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
//
}

VKGraphicsContext *VKGeneralCommandEncoder::GetVKContext() { 
    return (VKGraphicsContext*)_graphicsContext;
}

