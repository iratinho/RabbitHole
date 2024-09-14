#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKRenderPass.hpp"
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
#include "Renderer/Vendor/Vulkan/VKSamplerManager.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/Surface.hpp"
#include "Renderer/Fence.hpp"
#include "Renderer/Event.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"

#include "Renderer/CommandBuffer.hpp"

std::unordered_map<std::string, std::shared_ptr<VKGraphicsPipeline>> VKGraphicsContext::_pipelines;
std::unique_ptr<VKSamplerManager> VKGraphicsContext::_samplerManager;

VKGraphicsContext::VKGraphicsContext(RenderContext* renderContext)
    : _device(renderContext) {}

VKGraphicsContext::~VKGraphicsContext() {}

bool VKGraphicsContext::Initialize() {
    _fence = Fence::MakeFence({_device});
    _commandBuffer = CommandBuffer::MakeCommandBuffer({_device});
    
    // Currently we can always use the same command encoder, if we want to process
    // multiple render passes in parallel we might need multiple command encoders
    // and sync between them
    _commandEncoder = _commandBuffer->MakeRenderCommandEncoder(this, _device);
    _blitCommandEncoder = _commandBuffer->MakeBlitCommandEncoder(this, _device);

    _descriptorPool = _device->CreateDescriptorPool(1000, 1000);

    if(_descriptorPool == VK_NULL_HANDLE) {
        return false;
    }
    
    if(!_samplerManager) {
        _samplerManager = std::make_unique<VKSamplerManager>();
    }
    
    _descriptorsManager = std::make_unique<VKDescriptorManager>();
    
    return true;
}

void VKGraphicsContext::BeginFrame() {
    // Wait for the previous frame finish rendering
    _fence->Wait();
    
    _swapChainIndex = _device->GetSwapchain()->RequestNewPresentableImage();
        
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
    
    _descriptorsManager->ResetPools();
}

void VKGraphicsContext::Present() {
    _commandBuffer->Present(_swapChainIndex);
}

RenderContext* VKGraphicsContext::GetDevice() {
    return _device;
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
    if(node.GetType() == EGraphPassType::Raster) {
        const RasterNodeContext& passContext = node.GetContext<RasterNodeContext>();
        VKGraphicsPipeline* pipeline = static_cast<VKGraphicsPipeline*>(passContext._pipeline);
        if(!pipeline) {
            assert(0 && "Trying to execute render pass but pipline is invalid.");
            return;
        }

        _commandEncoder->BeginRenderPass(pipeline, passContext._renderAttachments);
        _commandEncoder->SetViewport(_device->GetSwapchainExtent()); // TODO get this from attachments 
        _commandEncoder->SetScissor(_device->GetSwapchainExtent(), {0, 0});
        
        passContext._callback(_commandEncoder, passContext._pipeline);
        
        _commandEncoder->EndRenderPass();
    }
    
    if(node.GetType() == EGraphPassType::Blit) {
        const BlitNodeContext& passContext = node.GetContext<BlitNodeContext>();
        passContext._callback(_blitCommandEncoder, passContext._readResources, passContext._writeResources);
    }
}
