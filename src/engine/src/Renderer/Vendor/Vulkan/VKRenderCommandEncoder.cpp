
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VulkanTranslator.hpp"
#include "Renderer/GPUDefinitions.h"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VKSamplerManager.hpp"
#include "Renderer/Vendor/Vulkan/VKTextureView.hpp"

void VKRenderCommandEncoder::BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) {
    auto* vkPipeline = dynamic_cast<VKGraphicsPipeline*>(pipeline);
    if(!pipeline) {
        assert(0 && "Unable to start vulkan render pass, pipeline is invalid.");
        return;
    }
    
    // TODO Ensure proper sync with read/write resources that might need to wait

    std::vector<Texture2D*> textures;
    Texture2D* colorTexture = attachments._colorAttachmentBinding->_texture.get();
    Texture2D* depthTexture = attachments._depthStencilAttachmentBinding.has_value() ? attachments._depthStencilAttachmentBinding->_texture.get() : nullptr;

    // Automatic layout transtion for attachments
    MakeImageBarrier(colorTexture, ImageLayout::LAYOUT_COLOR_ATTACHMENT);
    textures.emplace_back(colorTexture);

    if(depthTexture) {
        MakeImageBarrier(depthTexture, ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT);
        textures.emplace_back(depthTexture);
    }



    VkFramebuffer frameBuffer = vkPipeline->CreateFrameBuffer(textures);
    
    const float darkness = 0.28f;
    VkClearValue clearColor = {{{0.071435f * darkness, 0.079988f * darkness, 0.084369f * darkness, 1.0}}};
    VkClearValue clearDepth = {1.0f, 1.0f};
    std::array<VkClearValue, 2> clearValues = {clearColor, clearDepth};
    
    VkExtent2D extent;
    extent.width = colorTexture->GetWidth();
    extent.height = colorTexture->GetHeight();
    
    VkRenderPassBeginInfo beginPassInfo {};
    beginPassInfo.framebuffer = frameBuffer;
    beginPassInfo.pNext = nullptr;
    beginPassInfo.renderArea.extent = extent;
    beginPassInfo.renderArea.offset = {0, 0 };
    beginPassInfo.renderPass = vkPipeline->GetVKPass();
    beginPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginPassInfo.clearValueCount = clearValues.size();
    beginPassInfo.pClearValues = clearValues.data();
    
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdBeginRenderPass(commandBuffer, &beginPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkFunc::vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetVKPipeline());

    // TODO Ask DescriptorSetManager for the current descriptor set for the current in-flight frame
    // Bind shader descriptor sets
//    VkFunc::vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetVKPipelineLayout(), 0, 1, &, 0, nullptr);

}

void VKRenderCommandEncoder::EndRenderPass() {
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdEndRenderPass(commandBuffer);
}

void VKRenderCommandEncoder::SetViewport(const glm::vec2& viewportSize) {
    VkViewport viewport;
    viewport.height = viewportSize.y;
    viewport.width = viewportSize.x;
    viewport.x = 0;
    viewport.y = 0;
    viewport.maxDepth = 1;
    viewport.minDepth = 0;
    
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void VKRenderCommandEncoder::SetScissor(const glm::vec2& extent, const glm::vec2& offset) {
    VkRect2D scissor;
    scissor.offset = {(int32_t)offset.x, (int32_t)offset.y};
    scissor.extent = {(uint32_t)extent.x, (uint32_t)extent.y};
    
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VKRenderCommandEncoder::UpdatePushConstants(GraphicsPipeline* graphicsPipeline, Shader *shader, const void *data) {
    VKGraphicsPipeline* pipeline = (VKGraphicsPipeline*)graphicsPipeline;
    VKShader* vkShader = (VKShader*)shader;
    
    if(!pipeline || !shader || data == nullptr) {
        assert(0 && "Trying to upload a push constant with invalid parameters");
        return;
    }
    
    size_t offset;
    size_t size;
    
    if(vkShader->GetShaderStage() == ShaderStage::STAGE_VERTEX) {
        offset = vkShader->GetVertexConstantRange()->offset;
        size = vkShader->GetVertexConstantRange()->size;
    }
    
    if(vkShader->GetShaderStage() == ShaderStage::STAGE_FRAGMENT) {
        offset = vkShader->GetFragmentConstantRange()->offset;
        size = vkShader->GetFragmentConstantRange()->size;
    }

    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdPushConstants(commandBuffer, pipeline->GetVKPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | TranslateShaderStage(shader->GetShaderStage()), offset, size, data);
}

void VKRenderCommandEncoder::DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) {
    // TODO This seems out of place, we need API to SetVertexBuffer
    VKBuffer* buffer = (VKBuffer*)proxy._gpuBuffer.get();
    if(!buffer || (buffer && !buffer->GetLocalBuffer())) {
        assert(0 && "Invalid buffer for draw primitive indexed");
        return;
    }
        
    VkBuffer gpuBuffer = buffer->GetLocalBuffer();
        
    VkDeviceSize indicesOffset = proxy._indicesOffset;
    std::vector<VkDeviceSize> offsets = {proxy._vertexOffset};
        
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdBindIndexBuffer(commandBuffer, gpuBuffer, indicesOffset, VK_INDEX_TYPE_UINT32);
    VkFunc::vkCmdBindVertexBuffers(commandBuffer ,0, 1, &gpuBuffer, offsets.data());
    VkFunc::vkCmdDrawIndexed(commandBuffer, proxy._indicesCount, 1, 0, 0, 0);
}

void VKRenderCommandEncoder::Draw(std::uint32_t count) {
    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkFunc::vkCmdDraw(commandBuffer, count, 1, 0, 0);
}

void VKRenderCommandEncoder::MakeImageBarrier(Texture2D *texture2D, ImageLayout after) {
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

void VKRenderCommandEncoder::BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) {
    std::size_t hash = hash_value(resources);

    auto* context = dynamic_cast<VKGraphicsContext *>(_graphicsContext);
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

            VKTextureView* textureView = (VKTextureView*)resource._textureResource._texture->MakeTextureView();
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

    VkFunc::vkUpdateDescriptorSets(((VKDevice*)_graphicsContext->GetDevice())->GetLogicalDeviceHandle(), writes.size(), writes.data(), 0, VK_NULL_HANDLE);

    VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
    VkPipelineLayout layout = ((VKGraphicsPipeline*)shader->GetPipeline())->GetVKPipelineLayout();
    VkFunc::vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
}
