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
#include "Renderer/FrameResources.hpp"
#include "glm.hpp"

RenderPassExecutor::RenderPassExecutor(RenderContext *renderContext, CommandPool *commandPool, RenderPassGenerator &&generator, const std::string &passIdentifier, FrameResources *frameResources)
    : _renderContext(renderContext), _commandPool(commandPool), _generator(generator), _frameResources(frameResources), _passIdentifier(std::move(passIdentifier))
{
}

bool RenderPassExecutor::Execute(unsigned int frameIndex)
{
    PipelineStateObject *pso = _generator.Generate(_renderContext, frameIndex, _frameResources);

    if (!pso)
    {
        return false;
    }

    // Create framebuffer if its not valid at this point
    if (pso->framebuffer == VK_NULL_HANDLE)
    {
        unsigned int attachmentWidth = 0;
        unsigned int attachmentHeight = 0;
        std::vector<VkImageView> imageViews;
        for (AttachmentConfiguration &attachmentConfiguration : _generator._attachments)
        {
            imageViews.push_back((VkImageView)attachmentConfiguration._renderTarget->GetTexture()->GetView());

            // Assume that all attachments have same width and height
            attachmentWidth = attachmentConfiguration._renderTarget->GetWidth();
            attachmentHeight = attachmentConfiguration._renderTarget->GetHeight();
        }

        VkFramebufferCreateInfo framebufferCreateInfo{};
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

        if (result != VK_SUCCESS)
        {
            return false;
        }

        pso->framebuffer = framebuffer;
    }

    CommandBuffer *commandBuffer = _commandPool->GetCommandBuffer().get();
    //
    //    // Perform image layout transitions if necessary
    //    for (AttachmentConfiguration& attachmentConfiguration : _generator._attachments) {
    //        if(std::shared_ptr<RenderTarget> renderTarget = attachmentConfiguration._renderTarget) {
    //            if(std::shared_ptr<ITextureInterface> textureInterface = renderTarget->GetTexture()) {
    //                if(Texture* texture = static_cast<Texture*>(textureInterface.get())) {
    //                    std::cout <<  "Executing render pass with image layout as " << (int)texture->GetImageLayout() << std::endl;
    //
    //                    /*
    //                     * To avoid having vulkan concepts in the render pass structures, lets
    //                     * assume while we can that every pass will use color attachment optional and
    //                     * depth attachment.. Lets see if we need more granularity
    //                     */
    //
    //                    // Color attachment
    //                    if(texture->GetImageLayout() != attachmentConfiguration._initialLayout) {
    //                        VkImageMemoryBarrier barrier{};
    //                        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //                        barrier.oldLayout = TranslateImageLayout(texture->GetImageLayout());
    //                        barrier.newLayout = TranslateImageLayout(attachmentConfiguration._initialLayout);
    //                        barrier.image = (VkImage)texture->GetResource();
    //                        barrier.subresourceRange.baseMipLevel = 0;
    //                        barrier.subresourceRange.levelCount = 1;
    //                        barrier.subresourceRange.baseArrayLayer = 0;
    //                        barrier.subresourceRange.layerCount = 1;
    //                        barrier.subresourceRange.aspectMask = barrier.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
    //
    //                        VkFunc::vkCmdPipelineBarrier((VkCommandBuffer)commandBuffer->GetResource(),
    //                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // Source pipeline stage
    //                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // Destination pipeline stage
    //                            0,
    //                            0, nullptr,
    //                            0, nullptr,
    //                            1,
    //                            &barrier);
    //
    //                        std::cout <<  "Executing render pass and performed transition from " << (int)texture->GetImageLayout() << "to " << (int)attachmentConfiguration._initialLayout << std::endl;
    //
    //                        texture->SetImageLayout(attachmentConfiguration._initialLayout);
    //
    //
    //                    }
    //                }
    //            }
    //        }
    //    }

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
    //    std::cout << "\n\n@@@ NEW DRAW @@@" << std::endl;
    int primitiveIndex = 0;
    for (const PrimitiveProxy &primitiveData : _generator._primitiveData)
    {
        unsigned int pushConstantOffset = 0;

        // Bind all the vertex inputs
        if (!primitiveData._primitiveBuffer->GetResource())
            continue;

        for (const PushConstantConfiguration &pushConstantConfiguration : _generator._pushConstants)
        {
            if (pushConstantConfiguration._data.size() == 0)
            {
                assert(0);
            }

            std::vector<char> data;
            if (pushConstantConfiguration._data.size() - 1 >= primitiveIndex)
            {
                data = pushConstantConfiguration._data[primitiveIndex];
            }
            else
            {
                data = pushConstantConfiguration._data[0];
            }

            const size_t size = data.size() * sizeof(char);

            VkFunc::vkCmdPushConstants((VkCommandBuffer)commandBuffer->GetResource(), pso->pipeline_layout, TranslateShaderStage(pushConstantConfiguration._pushConstant._shaderStage), pushConstantOffset, size, data.data());

            pushConstantOffset += size;

            //            // Push constant debug
            //            std::printf("Primitive Number: %i \nPush Constant offset: %i \nSize %i\n", primitiveIndex, pushConstantOffset, size);
            //
            //            if(pushConstantConfiguration._debugType == "mat4") {
            //                std::cout << "Debug push constant data:" << std::endl;
            //                for (int i = 0; i < data.size(); i += sizeof(float)) {
            //                    std::cout << *reinterpret_cast<float*>(&data[i]) << std::endl;
            //                }
            //            }
        }

        primitiveIndex++;

        std::vector<VkDeviceSize> vertex_offsets;
        vertex_offsets.reserve(primitiveData._vOffset.size());
        for (const unsigned int offset : primitiveData._vOffset)
        {
            vertex_offsets.push_back(offset);
        }

        auto buffer = (VkBuffer)primitiveData._primitiveBuffer->GetResource();
        VkFunc::vkCmdBindVertexBuffers((VkCommandBuffer)commandBuffer->GetResource(), 0, 1, &buffer, vertex_offsets.data());

        VkFunc::vkCmdBindIndexBuffer((VkCommandBuffer)commandBuffer->GetResource(), (VkBuffer)primitiveData._primitiveBuffer->GetResource(), primitiveData._indicesBufferOffset, VK_INDEX_TYPE_UINT32);

        if (pso->descriptorSet)
        {
            VkFunc::vkCmdBindDescriptorSets((VkCommandBuffer)commandBuffer->GetResource(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso->pipeline_layout, 0, 1, &pso->descriptorSet, 0, nullptr);
        }

        // Issue Draw command
        VkFunc::vkCmdDrawIndexed((VkCommandBuffer)commandBuffer->GetResource(), primitiveData._indicesCount, 1, 0, 0, 0);
    }

    VkFunc::vkCmdEndRenderPass((VkCommandBuffer)commandBuffer->GetResource());

    return true;
}
