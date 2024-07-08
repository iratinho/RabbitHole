#include "Renderer/Vendor/Vulkan/VKRenderPass.hpp"

#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/VulkanLoader.hpp"

VKRenderPass::VKRenderPass(std::shared_ptr<VkGraphicsPipeline> pipeline)
    : _pipeline(pipeline) {}

void VKRenderPass::BeginRenderPass() {
    const float darkness = 0.28f;
    VkClearValue clearColor = {{{_renderPassValues.clearColor.x, _renderPassValues.clearColor.y, _renderPassValues.clearColor.z, 1.0}}};
    VkClearValue clearDepth = {_renderPassValues.clearDepth.x, _renderPassValues.clearDepth.y};
    std::array<VkClearValue, 2> clearValues = {clearColor, clearDepth};

    VkRenderPassBeginInfo renderPassBeginInfo {};
//    renderPassBeginInfo.framebuffer = pso->framebuffer;
    renderPassBeginInfo.pNext = nullptr;
//    renderPassBeginInfo.renderArea.extent = _renderContext->GetSwapchainExtent();
//    renderPassBeginInfo.renderArea.offset = {0, 0};
//    renderPassBeginInfo.renderPass = pso->render_pass;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
//    VkFunc::vkCmdBeginRenderPass((VkCommandBuffer)commandBuffer->GetResource(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VKRenderPass::EndRenderPass() {
//    VkFunc::vkCmdEndRenderPass((VkCommandBuffer)commandBuffer->GetResource());
}



