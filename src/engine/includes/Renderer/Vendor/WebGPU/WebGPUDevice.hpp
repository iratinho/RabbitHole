#pragma once
#include "webgpu/webgpu.hpp"
#include "Renderer/Device.hpp"

class WebGPUDevice : public Device {
public:
    bool Initialize() override;
    void Shutdown() override;

private:
    static WGPUAdapter CreateAdapter(WGPUInstance instance);

private:
    WGPUAdapter _adapter = nullptr;
};