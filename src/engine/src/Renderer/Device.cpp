#include "Renderer/Device.hpp"
#include "window.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#endif

std::unique_ptr<Device> Device::MakeDevice(Window* window) {
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKDevice>();
    instance->_window = window;
    return instance;
#endif
    
    return nullptr;
}

bool Device::Initialize() {
    if(!_window) {
        return false;
    }
    
    _renderContext = std::make_unique<RenderContext>();
    if(!_renderContext) {
        return false;
    }
    
    // TODO: Eventually we will not need the render context because it will be this Device
    const auto&[extensionCount, extensions] = _window->GetRequiredExtensions();
    
    const InitializationParams params {
            true,
            extensionCount,
            extensions,
            _window
    };
    
    if(!_renderContext->Initialize(params)) {
        return false;
    }
    
    _swapChain = std::make_unique<Swapchain>(_renderContext.get());
    if(!_swapChain) {
        return false;
    }
    
    if(!_swapChain->Initialize()) {
        return false;
    }
    
    const auto swapChainCount = _swapChain->GetSwapchainImageCount();
    for (int i = 0; i < swapChainCount; i++) {
        std::unique_ptr<GraphicsContext> context = GraphicsContext::Create(_renderContext.get());
        if(context) {
            if(!context->Initialize()) {
                return false;
            }
            _graphicsContexts.push_back(std::move(context));
        }
    }
    
    if(swapChainCount != _graphicsContexts.size()) {
        assert(0);
        return false;
    }
    
    return true;
}

GraphicsContext* Device::GetGraphicsContext(std::uint8_t idx) {
    if(idx >= 0 && idx < _graphicsContexts.size()) {
        return _graphicsContexts[idx].get();
    }
    
    assert(0);
    return nullptr;
}
