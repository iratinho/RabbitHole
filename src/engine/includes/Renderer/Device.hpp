#pragma once
#include "Renderer/Swapchain.hpp"
#include "Renderer/GraphicsContext.hpp"

class Window;

class Device {
public:
    virtual ~Device() = default;
    
    static std::unique_ptr<Device> MakeDevice(Window* window);
    
    virtual bool Initialize();
    virtual void Shutdown() = 0;
    
    // TEMPORARY RENDER CONTEXT
    RenderContext* GetRenderContext() { return _renderContext.get(); };
    
    GraphicsContext* GetGraphicsContext(std::uint8_t idx);
    
private:
    Window* _window = nullptr;
    std::unique_ptr<Swapchain> _swapChain;
    std::unique_ptr<RenderContext> _renderContext; // Temporary until this device is sorted out, will replace as baby steps
    std::vector<std::unique_ptr<GraphicsContext>> _graphicsContexts;
};
