#include "window.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKWindow.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUWindow.hpp"
#endif

std::unique_ptr<Window> Window::MakeWindow() {
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKWindow>();
    return instance;
#else
    auto instance = std::make_unique<WebGPUWindow>();
    return instance;
#endif

    return nullptr;
}

bool Window::Initialize(const WindowInitializationParams& params) {
    _params = params;
    
    _device = Device::MakeDevice(this);
    if(!_device) {
        return false;
    }
    
    bool bDeviceInitialized = _device->Initialize();
    if(!bDeviceInitialized) {
        return false;
    }

    _inputSystem = std::make_unique<InputSystem>(this);
        
    return true;
}

void Window::Shutdown() const {
    if(_device) {
        _device->Shutdown();
    }
}

bool Window::ShouldWindowClose() const noexcept {
    return true;
}

void Window::PoolEvents() {
    assert(0);
}

void Window::ClearDeltas()
{
    m_CurrentMouseDelta = glm::vec2(0.0f);
}

std::tuple<uint32_t, const char**> Window::GetRequiredExtensions() {
    assert(0);
    return {};
}

void * Window::GetWindow() const {
    assert(0);
    return nullptr;
}

void* Window::CreateSurface(void* instance) const {
    assert(0);
    return nullptr;
}

glm::i32vec2 Window::GetWindowSurfaceSize() const {
    return {};
}

void Window::HideCursor() const {
    assert(0);
}

void Window::ShowCursor() const {
    assert(0);
}
