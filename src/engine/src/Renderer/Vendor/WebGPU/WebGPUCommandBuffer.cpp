#include "Renderer/Vendor/WebGPU/WebGPUCommandBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPURenderCommandEncoder.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUBlitCommandEncoder.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUWindow.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUCommandEncoderSync.hpp"


namespace {
    template <typename T>
    T* CastEncoder(void* encoder) {
        return static_cast<T*>(encoder);
    }
}

bool WebGPUCommandBuffer::Initialize() {
    _device = reinterpret_cast<WebGPUDevice *>(_params._device);
    if(_device == nullptr) {
        return false;
    }

    _queue = wgpuDeviceGetQueue(_device->GetWebGPUDevice());
    // std::cout << "wgpuDeviceGetQueue" << std::endl;
    if(_queue == nullptr) {
        return false;
    }

    return true;
}

void WebGPUCommandBuffer::BeginRecording() {
}

void WebGPUCommandBuffer::EndRecording() {
}

void WebGPUCommandBuffer::Submit(std::shared_ptr<Fence> fence) {
    std::vector<WGPUCommandBuffer> commandBuffers;
    
    auto& sync = WebGPUCommandEncoderSync::GetInstance();
    
    WebGPUCommandEncoderSync::SyncCommandEncoderData encoderData = sync.GetNextCommandEncoder();
    while(encoderData._commandEncoder != nullptr) {
        WGPUCommandEncoder wgpuEncoder;
        std::string commandEncoderName = "";
        
        if(encoderData._tag == WebGPUCommandEncoderSync::SyncCommandEncoderTag::RenderCommandEncoder) {
            if(auto encoder = (WebGPURenderCommandEncoder*)encoderData._commandEncoder) {
                wgpuEncoder = encoder->GetWebGPUEncoder();
                commandEncoderName = "RENDER";
            }
        }
        
        if(encoderData._tag == WebGPUCommandEncoderSync::SyncCommandEncoderTag::BlitCommandEncoder) {
            if(auto encoder = (WebGPUBlitCommandEncoder*)encoderData._commandEncoder) {
                wgpuEncoder = encoder->GetWebGPUEncoder();
                commandEncoderName = "BLIT";
            }
        }

        if(wgpuEncoder) {
            WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
            cmdBufferDescriptor.nextInChain = nullptr;
            cmdBufferDescriptor.label = commandEncoderName.c_str();

            if(WGPUCommandBuffer command = wgpuCommandEncoderFinish(wgpuEncoder, &cmdBufferDescriptor)) {
                // std::cout << "wgpuCommandEncoderFinish (" << commandEncoderName << ")" << std::endl;
                commandBuffers.push_back(command);
            }
        }
        
        encoderData = sync.GetNextCommandEncoder();
    }
        
    wgpuQueueSubmit(_queue, commandBuffers.size(), commandBuffers.data());
    // std::cout << "wgpuQueueSubmit" << std::endl;
    
    // Release command buffers
    for (auto commandBuffer : commandBuffers) {
        wgpuCommandBufferRelease(commandBuffer);
        // std::cout << "wgpuCommandBufferRelease" << std::endl;
    }
}

void WebGPUCommandBuffer::Present() {
    // Presentation only makes sense on native, with emscripten the browser
    // is responsible to schedule rendering within its main loop
#ifndef __EMSCRIPTEN__
    
    WebGPUWindow* wgpuWindow = (WebGPUWindow*)(_device->GetWindow());
    WGPUSurface wgpuSurface = wgpuWindow ? wgpuWindow->GetWebGPUSurface() : nullptr;
    if(!wgpuSurface) {
        assert(false && "Invalid window surface during present");
        return false;
    }
    
    wgpuSurfacePresent(wgpuSurface);
    // std::cout << "wgpuSurfacePresent" << std::endl;
    
    // wgpuDevicePoll(_device->GetWebGPUDevice(), true, nullptr);

#endif
}

