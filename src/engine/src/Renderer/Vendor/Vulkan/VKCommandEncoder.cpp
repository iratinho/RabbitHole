
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/GPUDefinitions.h"

void VKCommandEncoder::SetViewport(GraphicsContext *graphicsContext, int width, int height) {
    VKGraphicsContext* context = (VKGraphicsContext*)graphicsContext;
    if(!context) {
        assert(0 && "Trying to set viewport with invalid parameters");
    }
    
    VkViewport viewport;
    viewport.height = (float)height;
    viewport.width = (float)width;
    viewport.x = 0;
    viewport.y = 0;
    viewport.maxDepth = 1;
    viewport.minDepth = 0;
    VkFunc::vkCmdSetViewport(context->GetCommandBuffer(), 0, 1, &viewport);
}

void VKCommandEncoder::UpdatePushConstants(GraphicsContext *graphicsContext, GraphicsPipeline* graphicsPipeline, Shader *shader, const void *data) {
    VKGraphicsContext* context = (VKGraphicsContext*)graphicsContext;
    VKGraphicsPipeline* pipeline = (VKGraphicsPipeline*)graphicsPipeline;
    VKShader* vkShader = (VKShader*)shader;
    
    if(!context || !pipeline || !shader || data == nullptr) {
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

    VkFunc::vkCmdPushConstants(context->GetCommandBuffer(), pipeline->GetVKPipelineLayout(), TranslateShaderStage(shader->GetShaderStage()), offset, size, data);
}

void VKCommandEncoder::DrawPrimitiveIndexed(GraphicsContext* graphicsContext, const PrimitiveProxyComponent& proxy) {
    if(VKGraphicsContext* context = (VKGraphicsContext*)graphicsContext) {
        
        VKBuffer* buffer = (VKBuffer*)proxy._gpuBuffer.get();
        if(!buffer || (buffer && !buffer->GetLocalBuffer())) {
            assert(0 && "Invalid buffer for draw primitive indexed");
            return;
        }
        
        
        VkBuffer gpuBuffer = buffer->GetLocalBuffer();
        
        VkDeviceSize indicesOffset = proxy._indicesOffset;
        std::vector<VkDeviceSize> offsets = {proxy._vertexOffset};
        
        VkFunc::vkCmdBindIndexBuffer(context->GetCommandBuffer(), gpuBuffer, indicesOffset, VK_INDEX_TYPE_UINT32);
        VkFunc::vkCmdBindVertexBuffers(context->GetCommandBuffer() ,0, 1, &gpuBuffer, offsets.data());
        VkFunc::vkCmdDrawIndexed(context->GetCommandBuffer(), proxy._indicesCount, 1, 0, 0, 0);
    }
}

