#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKRenderPass.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/Surface.hpp"
#include "Renderer/Fence.hpp"

namespace PrivUtils {
    std::pair<VkPipelineStageFlags, VkPipelineStageFlags> GetPipelineStageFlagsFromLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
        if(oldLayout == newLayout) {
            assert(0);
        }

        int error = 0;
        
        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;
        
        // PRESENT -> COLOR_ATTACHMENT / DEPTH_ATTACHMENT
        if(oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
            srcStage = newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dstStage = newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            
            error++;
        }
        
        // UNDEFINED -> COLOR_ATTACHMENT / DEPTH_ATTACHMENT
        if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            
            error++;
        }
        
        // COLOR_ATTACHMENT -> PRESENT
        if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            
            error++;
        }
        
        assert(error);
        
        return { srcStage, dstStage };
    }
    
    std::pair<VkAccessFlags, VkAccessFlags> GetAccessFlagsFromLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
        if(oldLayout == newLayout) {
            assert(0);
        }
        
        VkAccessFlags srcAccessFlags;
        VkAccessFlags dstAccessFlags;
        
        switch (oldLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                srcAccessFlags = 0;
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                srcAccessFlags = VK_ACCESS_MEMORY_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                srcAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            default:
                std::cerr << "Not handled layout for access flags translation. (oldLayout)" << std::endl;
        }
        
        switch (newLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                dstAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                dstAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                dstAccessFlags = VK_ACCESS_NONE;
                break;
            default:
                std::cerr << "Not handled layout for access flags translation. (newLayout)" << std::endl;
        }
        
        return { srcAccessFlags, dstAccessFlags };
    }
}

std::unordered_map<std::string, std::shared_ptr<VKGraphicsPipeline>> VKGraphicsContext::_pipelines;

// 1. Remove fence wrappers
// 2. Create a gpu event class (Metal= MTLEvent and Vulkan= VkSemaphore)
// 3. Create a fence abstraction (Metal= MTLFence and Vulkan= VkFence)
// 4. To better folow metal api lets make present function part of the command pool class
VKGraphicsContext::VKGraphicsContext(std::shared_ptr<RenderContext> renderContext)
    : _device(renderContext) {}

VKGraphicsContext::~VKGraphicsContext() {}

bool VKGraphicsContext::Initialize() {
    VkCommandPoolCreateInfo commandPoolInfo {};
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = _device->GetGraphicsQueueIndex();

    if (VkFunc::vkCreateCommandPool(_device->GetLogicalDeviceHandle(), &commandPoolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        return false;
    }
    
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.pNext = nullptr;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence = VK_NULL_HANDLE;
    if(VkFunc::vkCreateFence(_device->GetLogicalDeviceHandle(), &fenceInfo, nullptr, &_inFlightFence) != VK_SUCCESS) {
        return false;
    }
        
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.flags = 0;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = VkFunc::vkCreateSemaphore(_device->GetLogicalDeviceHandle(), &semaphoreInfo, nullptr, &_renderFinishedSemaphore);
    if(result != VK_SUCCESS) {
        return false;
    }
    
    _descriptorPool = _device->CreateDescriptorPool(1, 5);
    
    if(_descriptorPool == VK_NULL_HANDLE) {
        return false;
    }
    
    return true;
}

void VKGraphicsContext::BeginFrame() {
    // Wait for the previous frame finish rendering
    VkFunc::vkWaitForFences(_device->GetLogicalDeviceHandle(), 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    VkFunc::vkResetFences(_device->GetLogicalDeviceHandle(), 1, &_inFlightFence);

    // Ask to presentation engine for a new swapchain image index (Need to find better place for this.. it makes sense now because we always render to SC)
    _swapChainIndex = _device->GetSwapchain()->RequestNewPresentableImage();
    
    VkFunc::vkResetCommandPool(_device->GetLogicalDeviceHandle(), _commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

    VkCommandBufferAllocateInfo commandBufferInfo = {};
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandPool = static_cast<VkCommandPool>(_commandPool);
    commandBufferInfo.pNext = nullptr;
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandBufferCount = 1;

    if (VkFunc::vkAllocateCommandBuffers(_device->GetLogicalDeviceHandle(), &commandBufferInfo, &_commandBuffer) != VK_SUCCESS) {
        return;
    }
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = 0;
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;

    if(VkFunc::vkBeginCommandBuffer(static_cast<VkCommandBuffer>(_commandBuffer), &beginInfo) != VK_SUCCESS) {
        return;
    }
}

void VKGraphicsContext::EndFrame() {
    RenderTarget* renderTarget = _device->GetSwapchain()->GetSwapchainRenderTarget(ESwapchainRenderTargetType::COLOR, _swapChainIndex).get();
    
    VkImageLayout oldLayout = TranslateImageLayout(renderTarget->GetTexture()->GetCurrentLayout());
    VkImageLayout newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkImageSubresourceRange subresource;
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseMipLevel = 0;
    subresource.levelCount = 1;
    subresource.baseArrayLayer = 0;
    subresource.layerCount = 1;
    
    VkAccessFlags srcAccessMask, dstAccessMask;
    std::tie(srcAccessMask, dstAccessMask) = PrivUtils::GetAccessFlagsFromLayout(oldLayout, newLayout);
    
    VkTextureResource* textureResource = (VkTextureResource*)renderTarget->GetTexture()->GetResource().get();
    
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
    std::tie(srcStage, dstStage) = PrivUtils::GetPipelineStageFlagsFromLayout(oldLayout, newLayout);
    
    VkFunc::vkCmdPipelineBarrier(_commandBuffer, srcStage, dstStage, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

    if(VkFunc::vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
        return;
    }
    
    VkSemaphore semaphore = _device->GetSwapchain()->GetSyncPrimtiive(_swapChainIndex);
    
    VkSubmitInfo submitInfo{};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.pSignalSemaphores = &_renderFinishedSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    VkFunc::vkQueueSubmit(_device->GetGraphicsQueueHandle(), 1, &submitInfo, _inFlightFence);
}

void VKGraphicsContext::ExecutePipelines() {
}

void VKGraphicsContext::Present() {
    const auto swapChain = static_cast<VkSwapchainKHR>(_device->GetSwapchain()->GetNativeHandle());
    
    VkPresentInfoKHR present_info_khr;
    present_info_khr.pNext = nullptr;
    present_info_khr.pResults = nullptr;
    present_info_khr.pSwapchains = &swapChain;
    present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info_khr.swapchainCount = 1;
    present_info_khr.pImageIndices = &_swapChainIndex;
    present_info_khr.pWaitSemaphores = &_renderFinishedSemaphore;
    present_info_khr.waitSemaphoreCount = 1;
    VkFunc::vkQueuePresentKHR(_device->GetPresentQueueHandle(), &present_info_khr);
}

RenderContext* VKGraphicsContext::GetDevice() {
    return _device.get();
};

std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> VKGraphicsContext::GetPipelines() {
    std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> pipelines;
    for(auto& [name, pipeline] : _pipelines) {
        pipelines.push_back(std::make_pair(name, pipeline));
    }
    
    return pipelines;
}

std::shared_ptr<RenderTarget> VKGraphicsContext::GetSwapchainColorTarget() {
    if(_device && _device->GetSwapchain()) {
        return _device->GetSwapchain()->GetSwapchainRenderTarget(COLOR, _swapChainIndex);
    }
    
    return nullptr;
}

std::shared_ptr<RenderTarget> VKGraphicsContext::GetSwapchainDepthTarget() {
    if(_device && _device->GetSwapchain()) {
        return _device->GetSwapchain()->GetSwapchainRenderTarget(DEPTH, _swapChainIndex);
    }
    
    return nullptr;
}


void VKGraphicsContext::Execute(RenderGraphNode node) {
    RenderPassContext* passContext = node.GetContext();
    VKGraphicsPipeline* pipeline = static_cast<VKGraphicsPipeline*>(passContext->_pipeline);
    if(!pipeline) {
        return;
    }
        
    VkImageLayout oldColorImageLayout = TranslateImageLayout(passContext->_renderAttachments._colorAttachmentBinding->_renderTarget->GetTexture()->GetCurrentLayout());
    VkImageLayout oldDepthImageLayout = TranslateImageLayout(passContext->_renderAttachments._depthStencilAttachmentBinding->_renderTarget->GetTexture()->GetCurrentLayout());
    
    VkImageLayout newColorImageLayout = TranslateImageLayout(ImageLayout::LAYOUT_COLOR_ATTACHMENT);
    VkImageLayout newDepthImageLayout = TranslateImageLayout(ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT);
    
    if(newColorImageLayout != oldColorImageLayout) {
        VkImageSubresourceRange subresource;
        subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource.baseMipLevel = 0;
        subresource.levelCount = 1;
        subresource.baseArrayLayer = 0;
        subresource.layerCount = 1;
        
        VkAccessFlags srcAccessMask, dstAccessMask;
        std::tie(srcAccessMask, dstAccessMask) = PrivUtils::GetAccessFlagsFromLayout(oldColorImageLayout, newColorImageLayout);
        
        VkTextureResource* textureResource = (VkTextureResource*)passContext->_renderAttachments._colorAttachmentBinding->_renderTarget->GetTexture()->GetResource().get();
        
        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = VK_NULL_HANDLE;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.oldLayout = oldColorImageLayout;
        barrier.newLayout = newColorImageLayout;
        barrier.srcQueueFamilyIndex = 0;
        barrier.dstQueueFamilyIndex = 0;
        barrier.image = textureResource->GetImage();
        barrier.subresourceRange = subresource;
        
        VkPipelineStageFlags srcStage, dstStage;
        std::tie(srcStage, dstStage) = PrivUtils::GetPipelineStageFlagsFromLayout(oldColorImageLayout, newColorImageLayout);
        
        VkFunc::vkCmdPipelineBarrier(_commandBuffer, srcStage, dstStage, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
    }
    
    if(newDepthImageLayout != oldDepthImageLayout) {
        VkImageSubresourceRange subresource;
        subresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresource.baseMipLevel = 0;
        subresource.levelCount = 1;
        subresource.baseArrayLayer = 0;
        subresource.layerCount = 1;

        VkAccessFlags srcAccessMask, dstAccessMask;
        std::tie(srcAccessMask, dstAccessMask) = PrivUtils::GetAccessFlagsFromLayout(oldDepthImageLayout, newDepthImageLayout);
        
        VkTextureResource* textureResource = (VkTextureResource*)passContext->_renderAttachments._depthStencilAttachmentBinding->_renderTarget->GetTexture()->GetResource().get();

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = VK_NULL_HANDLE;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.oldLayout = oldDepthImageLayout;
        barrier.newLayout = newDepthImageLayout;
        barrier.srcQueueFamilyIndex = 0;
        barrier.dstQueueFamilyIndex = 0;
        barrier.image = textureResource->GetImage();
        barrier.subresourceRange = subresource;

        VkPipelineStageFlags srcStage, dstStage;
        std::tie(srcStage, dstStage) = PrivUtils::GetPipelineStageFlagsFromLayout(oldDepthImageLayout, newDepthImageLayout);
        
        VkFunc::vkCmdPipelineBarrier(_commandBuffer, srcStage, dstStage, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
    }
    
    passContext->_renderAttachments._colorAttachmentBinding->_renderTarget->GetTexture()->SetTextureLayout(ImageLayout::LAYOUT_COLOR_ATTACHMENT);
    passContext->_renderAttachments._depthStencilAttachmentBinding->_renderTarget->GetTexture()->SetTextureLayout(ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT);
    
    std::vector<RenderTarget*> renderTargets;
    renderTargets.emplace_back(passContext->_renderAttachments._colorAttachmentBinding->_renderTarget.get());
    renderTargets.emplace_back(passContext->_renderAttachments._depthStencilAttachmentBinding->_renderTarget.get());
    
    VkExtent2D extent;
    extent.width = passContext->_renderAttachments._colorAttachmentBinding->_renderTarget->GetWidth();
    extent.height = passContext->_renderAttachments._depthStencilAttachmentBinding->_renderTarget->GetHeight();
    
//    glm::vec3 colorClear = passContext->_colorTarget._clearColor;
//    glm::vec3 depthClear = passContext->_depthTarget._clearColor;
    
//    VkClearValue clearValues;
//    clearValues.color = {colorClear.r, colorClear.g, colorClear.b, 1.0f};
//    clearValues.depthStencil = {static_cast<float>((int)depthClear.x), static_cast<uint32_t>((int)depthClear.y)};
    
    const float darkness = 0.28f;
    VkClearValue clear_color = {{{0.071435f * darkness, 0.079988f * darkness, 0.084369f * darkness, 1.0}}};
    VkClearValue clear_depth = {1.0f, 1.0f};
    std::array<VkClearValue, 2> clearValues = {clear_color, clear_depth};
    
    VkFramebuffer frameBuffer = pipeline->CreateFrameBuffer(renderTargets);
    
    VkRenderPassBeginInfo beginPassInfo {};
    beginPassInfo.framebuffer = frameBuffer;
    beginPassInfo.pNext = nullptr;
    beginPassInfo.renderArea.extent = extent;
    beginPassInfo.renderArea.offset = {0, 0 };
    beginPassInfo.renderPass = pipeline->GetVKPass();
    beginPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginPassInfo.clearValueCount = clearValues.size();
    beginPassInfo.pClearValues = clearValues.data();
    
    VkFunc::vkCmdBeginRenderPass(_commandBuffer, &beginPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
    // Bind to graphics pipeline
    VkFunc::vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVKPipeline());
    
    // Viewport
    VkViewport viewport;
    viewport.height = (float)_device->GetSwapchainExtent().height;
    viewport.width = (float)_device->GetSwapchainExtent().width;
    viewport.x = 0;
    viewport.y = 0;
    viewport.maxDepth = 1;
    viewport.minDepth = 0;
    VkFunc::vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
    
    // Scissor
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = _device->GetSwapchainExtent();
    VkFunc::vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

    passContext->_callback(_commandEncoder.get(), passContext->_pipeline);

    VkFunc::vkCmdEndRenderPass(_commandBuffer);
}
