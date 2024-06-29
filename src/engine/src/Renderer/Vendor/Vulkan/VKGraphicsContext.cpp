#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKRenderPass.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/Surface.hpp"
#include "Renderer/Fence.hpp"
#include "Renderer/Event.hpp"

#include "Renderer/CommandBuffer.hpp"


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
    _fence = Fence::MakeFence({_device});
    _event = Event::MakeEvent({_device});
    _commandBuffer = CommandBuffer::MakeCommandBuffer({_device});
    _commandEncoder = _commandBuffer->MakeRenderCommandEncoder({_device});
    
    _descriptorPool = _device->CreateDescriptorPool(1, 5);

    if(_descriptorPool == VK_NULL_HANDLE) {
        return false;
    }
    
    return true;
}

void VKGraphicsContext::BeginFrame() {
    // Wait for the previous frame finish rendering
    _fence->Wait();
    
    _swapChainIndex = _device->GetSwapchain()->RequestNewPresentableImage();
    
    // Encodes an event that will be trigered on the GPU when the command buffer finishes execution
    _commandBuffer->EncodeSignalEvent(_event);
    
    // Encodes an event that will make our command buffer sumission wait for the swapchain image being ready
    std::shared_ptr<Event> waitEvent = _device->GetSwapchain()->GetSyncPrimtiive(_swapChainIndex);
    _commandBuffer->EncodeWaitForEvent(waitEvent);
    
    _commandBuffer->BeginRecording();
}

void VKGraphicsContext::EndFrame() {
    Texture2D* texture = _device->GetSwapchain()->GetSwapchainTexture(ESwapchainTextureType::COLOR, _swapChainIndex).get();
    _commandEncoder->MakeImageBarrier(texture, ImageLayout::LAYOUT_PRESENT);
    
    _commandBuffer->EndRecording();
    _commandBuffer->Submit(_fence);
}

void VKGraphicsContext::ExecutePipelines() {
}

void VKGraphicsContext::Present() {
    const auto swapChain = static_cast<VkSwapchainKHR>(_device->GetSwapchain()->GetNativeHandle());
    
    VkSemaphore semaphore = VK_NULL_HANDLE;
    std::shared_ptr<VKEvent> vkEvent = std::static_pointer_cast<VKEvent>(_event);
    if(vkEvent) {
        semaphore = vkEvent->GetVkSemaphore();
    }
    
    VkPresentInfoKHR present_info_khr;
    present_info_khr.pNext = nullptr;
    present_info_khr.pResults = nullptr;
    present_info_khr.pSwapchains = &swapChain;
    present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info_khr.swapchainCount = 1;
    present_info_khr.pImageIndices = &_swapChainIndex;
    present_info_khr.pWaitSemaphores = &semaphore;
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

std::shared_ptr<Texture2D> VKGraphicsContext::GetSwapChainColorTexture() {
    if(_device && _device->GetSwapchain()) {
        return _device->GetSwapchain()->GetSwapchainTexture(COLOR, _swapChainIndex);
    }
    
    return nullptr;
}

std::shared_ptr<Texture2D> VKGraphicsContext::GetSwapChainDepthTexture() {
    if(_device && _device->GetSwapchain()) {
        return _device->GetSwapchain()->GetSwapchainTexture(DEPTH, _swapChainIndex);
    }
    
    return nullptr;
}


void VKGraphicsContext::Execute(RenderGraphNode node) {
    
    if(node.GetType() != EGraphPassType::Raster) {
        assert(0 && "Trying to execute a non raster graph node in a raster queue!");
        return;
    }
    
    const RasterNodeContext& passContext = node.GetContext<RasterNodeContext>();
    VKGraphicsPipeline* pipeline = static_cast<VKGraphicsPipeline*>(passContext._pipeline);
    if(!pipeline) {
        assert(0 && "Trying to execute render pass but pipline is invalid.");
        return;
    }
            
    _commandEncoder->BeginRenderPass(pipeline, passContext._renderAttachments);
    _commandEncoder->SetViewport(_device->GetSwapchainExtent());
    _commandEncoder->SetScissor(_device->GetSwapchainExtent(), {0, 0});
    
    passContext._callback(_commandEncoder, passContext._pipeline);
    
    _commandEncoder->EndRenderPass();
}
