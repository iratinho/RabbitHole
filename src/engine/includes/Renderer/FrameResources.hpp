#pragma once
#include "Renderer/render_context.hpp"

class RenderContext;
class CommandPool;
class Surface;
class Fence;

// TODO later make this api independent
class FrameResources {
public:
    FrameResources() = delete;
    FrameResources(RenderContext* renderContext);
    
    bool Initialize();
    
    RenderContext* GetRenderContext() const {
        return _renderContext;
    }
    
    std::shared_ptr<CommandPool> GetCommandPool() const {
        return _commandPool;
    }
    
    std::shared_ptr<Surface> GetPresentableSurface() const {
        return _presentableSurface;
    }
    
    std::shared_ptr<Fence> GetInFlightFence() const {
        return _inFlightFence;
    }
    
    VkSemaphore GetRenderFinishedSemaphore() const {
        return _renderFinishedSemaphore;
    }
    
    VkDescriptorPool GetDescriptorPool() const {
        return _descriptorPool;
    }
    
private:
    RenderContext* _renderContext;
    std::shared_ptr<CommandPool> _commandPool;
    std::shared_ptr<Surface> _presentableSurface;
    std::shared_ptr<Fence> _inFlightFence;
    VkSemaphore _renderFinishedSemaphore;
    VkDescriptorPool _descriptorPool;
};

