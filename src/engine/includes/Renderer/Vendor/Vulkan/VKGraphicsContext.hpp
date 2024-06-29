#pragma once
#include "Renderer/GraphicsContext.hpp"
#include "vulkan/vulkan_core.h"

class VKGraphicsPipeline;
class RenderContext;
class CommandQueue;
class Surface;
class Fence;
class Event;
class CommandBuffer;
class Texture2D;

class VKGraphicsContext : public GraphicsContext {
public:
    VKGraphicsContext(std::shared_ptr<RenderContext> renderContext);
    ~VKGraphicsContext();
        
    bool Initialize() override;
    
    void BeginFrame() override;
    
    void EndFrame() override;
    
    void ExecutePipelines() override;
    
    void Present() override;
    
    RenderContext* GetDevice() override;
                
    std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> GetPipelines() override;
    
    std::shared_ptr<Texture2D> GetSwapChainColorTexture() override;
    
    std::shared_ptr<Texture2D> GetSwapChainDepthTexture() override;
        
    void Execute(RenderGraphNode node) override;
    
    VkDescriptorPool GetDescriptorPool() { return _descriptorPool; };
            
private:
    VkDescriptorPool _descriptorPool;
    unsigned int _swapChainIndex;
    std::shared_ptr<RenderContext> _device;
    
    static std::unordered_map<std::string, std::shared_ptr<VKGraphicsPipeline>> _pipelines; // Static, we want to be shared with other graphics context instances
    
    
    std::shared_ptr<Fence> _fence;
    std::shared_ptr<Event> _event;
    std::shared_ptr<CommandBuffer> _commandBuffer;
};
