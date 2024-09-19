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
        
    _swapChain = Swapchain::MakeSwapchain(this);
    if(!_swapChain) {
        return false;
    }
    
    if(!_swapChain->Initialize()) {
        return false;
    }
    
    const auto swapChainCount = _swapChain->GetImageCount();
    for (int i = 0; i < swapChainCount; i++) {
        if(std::unique_ptr<GraphicsContext> context = GraphicsContext::Create(this)) {
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

glm::vec2 Device::GetSwapchainExtent() const { 
    glm::vec2 extent;
    extent.x = static_cast<float>(GetWindow()->GetWindowSurfaceSize().x);
    extent.y = static_cast<float>(GetWindow()->GetWindowSurfaceSize().y);

    return extent;
}
