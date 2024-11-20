#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
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
    Texture2D* colorTexture = attachments._colorAttachmentBinding._texture.get();
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

void VKRenderCommandEncoder::DispatchDataStreams(GraphicsPipeline* graphicsPipeline, const std::vector<ShaderDataStream> dataStreams) {
    VKGraphicsPipeline* pipeline = (VKGraphicsPipeline*)graphicsPipeline;
    
    bool bBackendSupportsPushConstants = true;
    if(bBackendSupportsPushConstants) {
        VkShaderStageFlags stageFlags {};

        std::vector<const ShaderDataBlock*> vertexBlocks;
        std::vector<const ShaderDataBlock*> fragmentBlocks;
        
        // Compute necessary size for the push constant allocation and isolate data blocks
        std::size_t allocationSize = 0;
        for (const auto& dataStream : dataStreams) {
            if (dataStream._usage != ShaderDataStreamUsage::PUSH_CONSTANT) {
                continue;
            }
            
            for (const auto& block : dataStream._dataBlocks) {
                allocationSize += block._size;
                stageFlags |= TranslateShaderStage(block._stage);
                
                std::vector<const ShaderDataBlock*>& blockVectors = block._stage == ShaderStage::STAGE_VERTEX ? vertexBlocks : fragmentBlocks;
                blockVectors.push_back(&block);
            }
        }
        
        // Sort data streams, this sort will be vertex and then fragment (stable sort to preserve insertion order)
      /*  std::stable_sort(streams.begin(), streams.end(), [](const ShaderDataStream& a, const ShaderDataStream& b) {
            return (unsigned int)a._stage < (unsigned int)b._stage;
        });*/
        
        // Allocate buffer based on calculated sizes
        std::byte buffer[allocationSize];

        // Track current offset within each buffer
        std::size_t bytesCopied = 0;
        std::size_t fragmentDataStart = 0;

        // Copy the vertex data blocks
        for(const auto* block : vertexBlocks) {
            std::memcpy(buffer + bytesCopied, &block->_data, block->_size);
            bytesCopied += block->_size;
        }

        fragmentDataStart = bytesCopied;
        
        for(const auto* block : fragmentBlocks) {
            std::memcpy(buffer + bytesCopied, &block->_data, block->_size);
            bytesCopied += block->_size;
        }
        
        // Copy the data to our buffer
        /*for (const auto& dataStream : dataStreams) {
            
            if(dataStream._usage != ShaderDataStreamUsage::PUSH_CONSTANT) {
                continue;
            }
            
            if(dataStream._stage == ShaderStage::STAGE_FRAGMENT) {
                fragmentDataStart = bytesCopied;
            }
            
            if (dataStream._usage == ShaderDataStreamUsage::PUSH_CONSTANT) {
                for (const auto& block : dataStream._dataBlocks) {
                    std::memcpy(buffer + bytesCopied, &block._data, block._size);
                    bytesCopied += block._size;
                }
            }
        }
         */
        
        VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
        
        if(stageFlags & VK_SHADER_STAGE_VERTEX_BIT) {
            const auto dataSize = fragmentDataStart > 0 ? fragmentDataStart : allocationSize;

            VkFunc::vkCmdPushConstants(commandBuffer, pipeline->GetVKPipelineLayout(), stageFlags, 0, dataSize, buffer);
        }
        
        if(stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) {
            const auto dataSize = fragmentDataStart > 0 ? allocationSize - fragmentDataStart : allocationSize;
            const auto bufferPtr = fragmentDataStart > 0 ? buffer + fragmentDataStart : buffer;
            
            VkFunc::vkCmdPushConstants(commandBuffer, pipeline->GetVKPipelineLayout(), stageFlags, fragmentDataStart, dataSize, bufferPtr);
        }
    }
    
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
    
    std::vector<VkDescriptorSet> sets;
    
    for(const auto& dataStream : dataStreams) {
        if(dataStream._usage == ShaderDataStreamUsage::PUSH_CONSTANT) {
            continue;
        }
        
        VkDescriptorSet descriptorSet = descriptorManager->AcquireDescriptorSet(_graphicsContext, dataStream);

        if(descriptorSet == VK_NULL_HANDLE) {
            assert(0);
            return;
        }
        
        sets.push_back(descriptorSet);
        
        std::vector<VkWriteDescriptorSet> writes;
        
        unsigned int binding = 0;
        for(const auto& block : dataStream._dataBlocks) {
            VkWriteDescriptorSet writeDescriptor {};
            writeDescriptor.dstSet = descriptorSet;
            writeDescriptor.dstBinding = binding;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            if(block._usage == ShaderDataBlockUsage::UNIFORM_BUFFER) {
                if(!std::holds_alternative<ShaderBufferResource>(block._data)) {
                    assert(0);
                    continue;
                }
                
                VKBuffer* vkBuffer = (VKBuffer*)std::get<ShaderBufferResource>(block._data)._bufferResource.get();
                if(!vkBuffer) {
                    assert(0);
                    continue;
                }
                
                VkDescriptorBufferInfo bufferInfo {};
                bufferInfo.offset = std::get<ShaderBufferResource>(block._data)._offset;
                bufferInfo.range = vkBuffer->GetSize() - bufferInfo.offset;
                bufferInfo.buffer = vkBuffer->GetHostBuffer();
                
                writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptor.pBufferInfo = &bufferInfo;
                
                vkBuffer->ClearDirty();
            }
            
            if(block._usage == ShaderDataBlockUsage::TEXTURE) {
                if(!std::holds_alternative<ShaderTextureResource>(block._data)) {
                    assert(0);
                    continue;
                }
                
                ShaderTextureResource shaderTextureResource = std::get<ShaderTextureResource>(block._data);
                Texture2D* texture2D = shaderTextureResource._texture.get();
                VkTextureResource* textureResource = texture2D ? (VkTextureResource*)texture2D->GetResource().get() : nullptr;
                
                if(!textureResource) {
                    //assert(0);
                    continue;
                }
                
                VKTextureView* textureView = (VKTextureView*)texture2D->MakeTextureView();
                if(!textureView) {
                    assert(0);
                    continue;
                }
                
                VkSampler sampler = samplerManager->AcquireSampler(_graphicsContext, shaderTextureResource._sampler);
                if(sampler == VK_NULL_HANDLE) {
                    assert(0);
                    continue;
                }

                VkDescriptorImageInfo imageInfo {};
                imageInfo.imageView = textureView->GetImageView();
                imageInfo.imageLayout = TranslateImageLayout(texture2D->GetCurrentLayout());
                imageInfo.sampler = sampler;

                writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptor.pImageInfo = &imageInfo;
            }
            
            writes.push_back(writeDescriptor);
        }
        
        VkFunc::vkUpdateDescriptorSets(((VKDevice*)_graphicsContext->GetDevice())->GetLogicalDeviceHandle(), writes.size(), writes.data(), 0, VK_NULL_HANDLE);
    }
    
    if(!sets.empty()) {
        VkCommandBuffer commandBuffer = ((VKCommandBuffer*)_commandBuffer)->GetVkCommandBuffer();
        VkPipelineLayout layout = ((VKGraphicsPipeline*)pipeline)->GetVKPipelineLayout();
        VkFunc::vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, sets.size(), sets.data(), 0, nullptr);
    }
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

void VKRenderCommandEncoder::UploadBuffer(std::shared_ptr<Buffer> buffer) {
    VKBuffer* vkBuffer = dynamic_cast<VKBuffer*>(buffer.get());
    
    if(vkBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = buffer->GetSize();
        
        VkCommandBuffer commandBuffer = dynamic_cast<VKCommandBuffer *>(_commandBuffer)->GetVkCommandBuffer();
        VkFunc::vkCmdCopyBuffer(commandBuffer, vkBuffer->GetHostBuffer(), vkBuffer->GetLocalBuffer(), 1, &copyRegion);
    }
}

void VKRenderCommandEncoder::UploadImageBuffer(std::shared_ptr<Texture2D> texture) {
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
