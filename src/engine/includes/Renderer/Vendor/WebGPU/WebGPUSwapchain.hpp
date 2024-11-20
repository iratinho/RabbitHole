#pragma once
#include "Renderer/Swapchain.hpp"
#include "webgpu/webgpu.hpp"

class Texture2D;

class WebGPUSwapchain : public Swapchain {
public:
    explicit WebGPUSwapchain(Device *device)
        : Swapchain(device) {
    }

    bool Initialize() override;
    void Shutdown() override;
    bool PrepareNextImage() override;
    std::shared_ptr<Texture2D> GetTexture(ESwapchainTextureType_ type) override;
    std::shared_ptr<Event> GetSyncEvent() override;

private:
    void Recreate();

private:
    std::shared_ptr<Texture2D> _colorTexture;
    std::shared_ptr<Texture2D> _depthTexture;

    WGPUSurfaceTexture surfaceTexture;
};
