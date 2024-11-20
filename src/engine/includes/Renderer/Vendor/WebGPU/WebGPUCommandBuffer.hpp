#pragma once
#include "Renderer/CommandBuffer.hpp"
#include "webgpu/webgpu.hpp"

class WebGPUDevice;

class WebGPUCommandBuffer : public CommandBuffer {
public:
    void BeginRecording() override;

    void EndRecording() override;

    void Submit(std::shared_ptr<Fence> fence) override;

    void Present(uint32_t swapChainIndex) override;

protected:
    bool Initialize() override;

private:
    WGPUQueue _queue = nullptr;
    WebGPUDevice *_device = nullptr;
};