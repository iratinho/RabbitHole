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
    
    // TODO Swapchaina sks for this, for we should pass in the constructor
    [[nodiscard]] Window* GetWindow() const { return _window; }
        
    GraphicsContext* GetGraphicsContext(std::uint8_t idx);
    
    [[nodiscard]] Swapchain* GetSwapchain() const { return _swapChain.get(); }
    
    [[nodiscard]] glm::vec2 GetSwapchainExtent() const;
    
private:
    Window* _window = nullptr;
    std::unique_ptr<Swapchain> _swapChain;
    std::vector<std::unique_ptr<GraphicsContext>> _graphicsContexts;
};
