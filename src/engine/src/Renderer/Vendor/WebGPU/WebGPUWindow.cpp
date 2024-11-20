#include "Renderer/Vendor/WebGPU/WebGPUWindow.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "glfw3webgpu/glfw3webgpu.h"
#include "webgpu/webgpu.hpp"

bool WebGPUWindow::Initialize(const WindowInitializationParams &params) {
    // TODO: Make desktop window and web window share this logic since glfw is agnostic
//    if(!glfwInit()) {
//        //const int code = glfwGetError(nullptr);
//        throw std::runtime_error("[Error]: Failed to initialize glfw3 library.");
//        return false;
//    }
//
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    _window = glfwCreateWindow(params.width_, params.height_,
//                               params.title_, nullptr, nullptr);

    return DesktopWindow::Initialize(params);
}

bool WebGPUWindow::ShouldWindowClose() const noexcept {
    return false; // It should be browser call to decide that, make it always false
}

void WebGPUWindow::PoolEvents() {
    DesktopWindow::PoolEvents();
}

void * WebGPUWindow::CreateSurface(void *instance) {
    if(!_window) {
        assert(0 && "WebGPUWindow::CreateSurface invalid window.");
        return nullptr;
    }

    auto* device = reinterpret_cast<WebGPUDevice *>(GetDevice());
    if(!device) {
        assert(0 && "WebGPUWindow::CreateSurface failed");
        return nullptr;
    }

    WGPUSurfaceDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.label = "WebGPUWindowSurface";

    _surface = glfwGetWGPUSurface(device->GetWebGPUInstance(), _window);
    if(!_surface) {
        assert(0 && "WebGPUWindow::CreateSurface failed");
        return nullptr;
    }
    
    std::array<WGPUCompositeAlphaMode, 1> alphaModes { WGPUCompositeAlphaMode_Auto };

    WGPUSurfaceConfiguration config = {};
    config.nextInChain = nullptr;
    config.width = _params.width_;
    config.height = _params.height_;
    config.device = device->GetWebGPUDevice();
    config.format = wgpuSurfaceGetPreferredFormat(_surface, device->GetWebGPUAdapter());
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;

    wgpuSurfaceConfigure(_surface, &config);

    return _surface;
}
