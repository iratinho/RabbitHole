#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "webgpu/webgpu.hpp"

bool WebGPUDevice::Initialize() {
    // Descriptor is not yet implemented in the browser implementation
    WGPUInstance instance = wgpuCreateInstance(nullptr);
    if (!instance)
        return false;

    _adapter = CreateAdapter(instance);
    if (!_adapter)
        return false;

    return true;
}

void WebGPUDevice::Shutdown() {
    if(_adapter) {
        wgpuAdapterRelease(_adapter);
    }
}

WGPUAdapter WebGPUDevice::CreateAdapter(WGPUInstance instance) {
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };

    UserData userData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * pUserData) {
        UserData& userData = *static_cast<UserData*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuInstanceRequestAdapter(
        instance,
        nullptr,
        onAdapterRequestEnded,
        &userData);

#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    assert(userData.requestEnded);

    return userData.adapter;
}
