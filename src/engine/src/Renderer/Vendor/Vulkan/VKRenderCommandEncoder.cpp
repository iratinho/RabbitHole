
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/GPUDefinitions.h"
#include "Renderer/Texture2D.hpp"

void VKRenderCommandEncoder::BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) {
    VKGraphicsPipeline* vkPipeline = static_cast<VKGraphicsPipeline*>(pipeline);
    if(!pipeline) {
        assert(0 && "Unable to start vulkan render pass, pipeline is invalid.");
        return;
    }
    
    // TODO Ensure proper sync with read/write resources that might need to wait
    
    Texture2D* colorTexture = attachments._colorAttachmentBinding->_texture.get();
    Texture2D* depthTexture = attachments._depthStencilAttachmentBinding->_texture.get();

    // Automatic layout transtion for attachments
    MakeImageBarrier(colorTexture, ImageLayout::LAYOUT_COLOR_ATTACHMENT);
    MakeImageBarrier(depthTexture, ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT);

    std::vector<Texture2D*> textures;
    textures.emplace_back(attachments._colorAttachmentBinding->_texture.get());
    textures.emplace_back(attachments._depthStencilAttachmentBinding->_texture.get());
            
    VkFramebuffer frameBuffer = vkPipeline->CreateFrameBuffer(textures);
    
    const float darkness = 0.28f;
    VkClearValue clearColor = {{{0.071435f * darkness, 0.079988f * darkness, 0.084369f * darkness, 1.0}}};
    VkClearValue clearDepth = {1.0f, 1.0f};
    std::array<VkClearValue, 2> clearValues = {clearColor, clearDepth};
    
    VkExtent2D extent;
    extent.width = attachments._colorAttachmentBinding->_texture->GetWidth();
    extent.height = attachments._depthStencilAttachmentBinding->_texture->GetHeight();
    
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
    VkFunc::vkCmdPushConstants(commandBuffer, pipeline->GetVKPipelineLayout(), TranslateShaderStage(shader->GetShaderStage()), offset, size, data);
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
