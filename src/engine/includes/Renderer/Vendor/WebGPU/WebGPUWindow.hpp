#pragma once
#include "webgpu/webgpu.hpp"
#include "window.hpp"
#include "Window/Desktop/DesktopWindow.hpp"

class WebGPUWindow : public DesktopWindow {
public:
    bool Initialize(const WindowInitializationParams &params) override;
    bool ShouldWindowClose() const noexcept override;
    void PoolEvents() override;
    void * CreateSurface(void *instance) override;

    [[nodiscard]] WGPUSurface GetWebGPUSurface() const {
        return _surface;
    }

private:
    WGPUSurface _surface = nullptr;
};
