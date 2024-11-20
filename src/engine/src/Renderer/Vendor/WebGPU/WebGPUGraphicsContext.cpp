#include "Renderer/Vendor/WebGPU/WebGPUGraphicsContext.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/Device.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "webgpu/webgpu.hpp"

WebGPUGraphicsContext::WebGPUGraphicsContext(Device *device) {
    _device = device;
}

bool WebGPUGraphicsContext::Initialize() {
    _commandBuffer = CommandBuffer::MakeCommandBuffer({_device});

    return true;
}

void WebGPUGraphicsContext::BeginFrame() {
    std::cout << "---- BEGIN FRAME ----" << std::endl;
    if(_commandEncoder) {
        _commandBuffer->RemoveEncoder(_commandEncoder);
    }
    
    // In WebGPU we have new encoders per frame
    _commandEncoder = _commandBuffer->MakeRenderCommandEncoder(this, _device);

    // TODO create a blit command encoder....

    Swapchain* swapchain = _device ? _device->GetSwapchain() : nullptr;
    if(!swapchain) {
        std::cerr << "Failed to get swap chain" << std::endl;
        return;

    }

    if(!swapchain->PrepareNextImage()) {
        std::cerr << "Failed to prepare swapchain" << std::endl;
        return;
    }

    _commandBuffer->BeginRecording();
}

void WebGPUGraphicsContext::EndFrame() {
    _commandBuffer->EndRecording();
    _commandBuffer->Submit(nullptr);
    
    std::cout << "---- BEGIN FRAME ----" << std::endl;


#if defined(WEBGPU_BACKEND) && !defined(__EMSCRIPTEN__)
    auto device = reinterpret_cast<WebGPUDevice *>(_device);
    if(!device) {
        std::cerr << "Failed to get device" << std::endl;
        return;
    }

    wgpuDevicePoll(device->GetWebGPUDevice(), false, nullptr);
#endif

#if defined(__EMSCRIPTEN__)
    emscripten_sleep(1000);
#endif
}

std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> WebGPUGraphicsContext::GetPipelines() {
    return {};
}

std::shared_ptr<Texture2D> WebGPUGraphicsContext::GetSwapChainColorTexture() {
    Swapchain* swapchain = _device ? _device->GetSwapchain() : nullptr;
    if(!swapchain) {
        std::cerr << "Failed to get swap chain" << std::endl;
        return nullptr;
    }

    return _device->GetSwapchain()->GetTexture(_COLOR);
}

std::shared_ptr<Texture2D> WebGPUGraphicsContext::GetSwapChainDepthTexture() {
    Swapchain* swapchain = _device ? _device->GetSwapchain() : nullptr;
    if(!swapchain) {
        std::cerr << "Failed to get swap chain" << std::endl;
        return nullptr;
    }

    return _device->GetSwapchain()->GetTexture(_DEPTH);
}

void WebGPUGraphicsContext::Present() {
    if(!_commandBuffer) {
        std::cerr << "Failed to present command buffer" << std::endl;
        return;
    }

    _commandBuffer->Present(0);
}

void WebGPUGraphicsContext::Execute(RenderGraphNode node) {
    if(node.GetType() == EGraphPassType::Raster) {
        const RasterNodeContext& passContext = node.GetContext<RasterNodeContext>();

        if(!passContext._pipeline) {
            std::cerr << "Failed to execute pipeline" << std::endl;
            return;
        }

        _commandEncoder->BeginRenderPass(passContext._pipeline, passContext._renderAttachments);
        _commandEncoder->SetViewport(_device->GetSwapchainExtent()); // TODO get this from attachments
        _commandEncoder->SetScissor(_device->GetSwapchainExtent(), {0, 0});

        Encoders encoders {};
        encoders._renderEncoder = _commandEncoder;

        passContext._callback(encoders, passContext._pipeline);

        _commandEncoder->EndRenderPass();
    }

    if(node.GetType() == EGraphPassType::Blit) {
        Encoders encoders {};
        encoders._renderEncoder = _commandEncoder;
        const BlitNodeContext& passContext = node.GetContext<BlitNodeContext>();
        passContext._callback(encoders, passContext._readResources, passContext._writeResources);
    }
}
