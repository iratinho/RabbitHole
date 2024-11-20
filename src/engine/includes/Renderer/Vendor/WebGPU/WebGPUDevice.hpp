#pragma once
#include "webgpu/webgpu.hpp"
#include "Renderer/Device.hpp"

class WebGPUDevice : public Device {
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };


public:
    bool Initialize() override;
    void Shutdown() override;

    [[nodiscard]] WGPUAdapter GetWebGPUAdapter() const {
        return _adapter;
    }

    [[nodiscard]] WGPUDevice GetWebGPUDevice() const {
        return _device;
    }

    [[nodiscard]] WGPUInstance GetWebGPUInstance() const {
        return _instance;
    }

private:
    WGPUAdapter CreateAdapter(WGPUInstance instance);
    static WGPUSupportedLimits GetAdapterSupportedLimits(WGPUAdapter adapter);
    static std::vector<WGPUFeatureName> GetAdapterSupportedFeatures(WGPUAdapter adapter);
    static WGPUAdapterProperties GetAdapterProperties(WGPUAdapter adapter);

    WGPUDevice CreateDevice(WGPUAdapter adapter);
    static void ReportDevice(WGPUDevice device);

    void WaitForRequest();
private:
    WGPUInstance _instance = nullptr;
    WGPUAdapter _adapter = nullptr;
    WGPUDevice _device = nullptr;

    UserData userData;

};