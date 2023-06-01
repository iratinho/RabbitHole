#include "Renderer/RenderPass/RenderPassExecutor.hpp"
#include "Renderer/RenderSystem.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/RenderGraph/RenderGraph.hpp"
#include "Renderer/CommandPool.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Core/Components/MeshComponent.hpp"
#include "Core/GeometryLoaderSystem.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "glm.hpp"

RenderPassExecutor::RenderPassExecutor(RenderContext* renderContext, CommandPool* commandPool, RenderPassGenerator&& generator)
    : _renderContext(renderContext)
    , _commandPool(commandPool)
    , _generator(generator)
{
}

bool RenderPassExecutor::Execute(unsigned int frameIndex) {
    PipelineStateObject* pso = _generator.Generate(_renderContext, frameIndex);

    if(!pso) {
        return false;
    }

    // Create framebuffer if its not valid at this point
    if(pso->framebuffer == VK_NULL_HANDLE) {
        unsigned int attachmentWidth = 0;
        unsigned int attachmentHeight = 0;
        std::vector<VkImageView> imageViews;
        for (AttachmentConfiguration& attachmentConfiguration : _generator._attachments) {
            imageViews.push_back((VkImageView)attachmentConfiguration._renderTarget->GetView());

            // Assume that all attachments have same width and height
            attachmentWidth = attachmentConfiguration._renderTarget->GetWidth();
            attachmentHeight = attachmentConfiguration._renderTarget->GetHeight();
        }

        VkFramebufferCreateInfo framebufferCreateInfo {};
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.height = attachmentHeight;
        framebufferCreateInfo.layers = 1;
        framebufferCreateInfo.width = attachmentWidth;
        framebufferCreateInfo.attachmentCount = imageViews.size();
        framebufferCreateInfo.pAttachments = imageViews.data();
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.renderPass = pso->render_pass;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        VkFramebuffer framebuffer;
        const VkResult result = VkFunc::vkCreateFramebuffer(_renderContext->GetLogicalDeviceHandle(), &framebufferCreateInfo, nullptr, &framebuffer);

        if(result != VK_SUCCESS) {
            return false;
        }

        pso->framebuffer = framebuffer;
    }

    CommandBuffer* commandBuffer = _commandPool->GetCommandBuffer().get();

    // TODO add support for clear colors in attachment configuration
    // Clear color values for color and depth
    const float darkness = 0.28f;
    VkClearValue clear_color = {{{0.071435f * darkness, 0.079988f * darkness, 0.084369f * darkness, 1.0}}};
    VkClearValue clear_depth = {1.0f, 0.0f};
    std::array<VkClearValue, 2> clear_values = {clear_color, clear_depth};

    // Render pass
    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.framebuffer = pso->framebuffer;
    render_pass_begin_info.pNext = nullptr;
    render_pass_begin_info.renderArea.extent = _renderContext->GetSwapchainExtent();
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderPass = pso->render_pass;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.clearValueCount = clear_values.size();
    render_pass_begin_info.pClearValues = clear_values.data();
    VkFunc::vkCmdBeginRenderPass((VkCommandBuffer)commandBuffer->GetResource(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    // Bind to graphics pipeline
    VkFunc::vkCmdBindPipeline((VkCommandBuffer)commandBuffer->GetResource(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso->pipeline);

    // Viewport
    VkViewport viewport;
    viewport.height = (float)_renderContext->GetSwapchainExtent().height;
    viewport.width = (float)_renderContext->GetSwapchainExtent().width;
    viewport.x = 0;
    viewport.y = 0;
    viewport.maxDepth = 1;
    viewport.minDepth = 0;

    VkFunc::vkCmdSetViewport((VkCommandBuffer)commandBuffer->GetResource(), 0, 1, &viewport);

    // Scissor
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = _renderContext->GetSwapchainExtent();

    VkFunc::vkCmdSetScissor((VkCommandBuffer)commandBuffer->GetResource(), 0, 1, &scissor);

    // Draw call per primitive
    for (const PrimitiveProxy& primitiveData : _generator._primitiveData) {
        // Bind all the vertex inputs
        if(!primitiveData._primitiveBuffer->GetResource())
            continue;

        std::vector<VkDeviceSize> vertex_offsets;
        vertex_offsets.reserve(primitiveData._vOffset.size());
        for(const unsigned int offset : primitiveData._vOffset) {
            vertex_offsets.push_back(offset);
        }

        auto buffer = (VkBuffer)primitiveData._primitiveBuffer->GetResource();
        VkFunc::vkCmdBindVertexBuffers((VkCommandBuffer)commandBuffer->GetResource()
                ,0
                , 1
                , &buffer
                , vertex_offsets.data());

        /*
        for(const InputDescriptor& inputDescriptor : primitiveData._inputDescriptors) {
            if(!inputDescriptor._bEnabled) {
                continue;
            }
            
            const VkDeviceSize vertex_offsets = inputDescriptor._bufferOffset;
            auto buffer = (VkBuffer)primitiveData._primitiveBuffer->GetResource();
            VkFunc::vkCmdBindVertexBuffers((VkCommandBuffer)commandBuffer->GetResource()
                                           ,inputDescriptor._binding
                                           ,1
                                           , &buffer
                                           , &vertex_offsets);
        }
         */

        for (PushConstantConfiguration& pushConstantConfiguration : _generator._pushConstants) {
            auto matrix = std::any_cast<glm::mat4>(pushConstantConfiguration._data);

            VkFunc::vkCmdPushConstants((VkCommandBuffer)commandBuffer->GetResource()
                                       , pso->pipeline_layout
                                       , TranslateShaderStage(pushConstantConfiguration._pushConstant._shaderStage)
                                       , 0
                                       , pushConstantConfiguration._pushConstant._size
                                       , &matrix);
        }

        VkFunc::vkCmdBindIndexBuffer((VkCommandBuffer)commandBuffer->GetResource()
                                     ,(VkBuffer)primitiveData._primitiveBuffer->GetResource()
                                     ,primitiveData._indicesBufferOffset
                                     ,VK_INDEX_TYPE_UINT32);

        // Issue Draw command
        VkFunc::vkCmdDrawIndexed((VkCommandBuffer)commandBuffer->GetResource()
                                 ,primitiveData._indicesCount
                                 ,1
                                 ,0
                                 ,0
                                 ,0);
    }

    VkFunc::vkCmdEndRenderPass((VkCommandBuffer)commandBuffer->GetResource());

    return true;
}
