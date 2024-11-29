#pragma once
#include "Renderer/CommandBuffer.hpp"
#include "webgpu/webgpu.hpp"

class WebGPUDevice;

class WebGPUCommandBuffer : public CommandBuffer {
public:
    using Type = WebGPUCommandBuffer;

    void BeginRecording() override;

    void EndRecording() override;

    void Submit(std::shared_ptr<Fence> fence) override;

    void Present() override;

protected:
    bool Initialize() override;

private:
    WGPUQueue _queue = nullptr;
    WebGPUDevice *_device = nullptr;
};
