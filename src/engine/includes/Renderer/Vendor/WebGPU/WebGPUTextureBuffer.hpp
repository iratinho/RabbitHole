#pragma once
#include "Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"

class WebGPUTextureBuffer : public WebGPUBuffer {
public:
    WebGPUTextureBuffer(Device* device, std::weak_ptr<TextureResource> resource)
    {
        _device = device;
    }
};
