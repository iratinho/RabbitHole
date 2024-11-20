#include "Renderer/Vendor/WebGPU/WebGPUCommandBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPURenderCommandEncoder.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUWindow.hpp"

bool WebGPUCommandBuffer::Initialize() {
    _device = reinterpret_cast<WebGPUDevice *>(_params._device);
    if(_device == nullptr) {
        return false;
    }

    _queue = wgpuDeviceGetQueue(_device->GetWebGPUDevice());
    std::cout << "wgpuDeviceGetQueue" << std::endl;
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

    // Collect render command encoders
    for (auto& renderCommandEncoder: _renderCommandEncoders) {
        if(auto wgpuCommandEncoder = reinterpret_cast<WebGPURenderCommandEncoder *>(renderCommandEncoder.get()) ) {
            if(WGPUCommandEncoder encoder = wgpuCommandEncoder->GetWebGPUEncoder()) {
                WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
                cmdBufferDescriptor.nextInChain = nullptr;
                cmdBufferDescriptor.label = "Command buffer";

                if(WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor)) {
                    std::cout << "wgpuCommandEncoderFinish" << std::endl;
                    commandBuffers.push_back(command);
                }
            }
        }
    }

    wgpuQueueSubmit(_queue, commandBuffers.size(), commandBuffers.data());
    std::cout << "wgpuQueueSubmit" << std::endl;
    
    // Release command buffers
    for (auto commandBuffer : commandBuffers) {
        wgpuCommandBufferRelease(commandBuffer);
        std::cout << "wgpuCommandBufferRelease" << std::endl;
    }
}

void WebGPUCommandBuffer::Present(uint32_t swapChainIndex) {
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
    std::cout << "wgpuSurfacePresent" << std::endl;
    
    // wgpuDevicePoll(_device->GetWebGPUDevice(), true, nullptr);

#endif
}

