#pragma once
#include "Renderer/GraphicsContext.hpp"
#include "vulkan/vulkan_core.h"

class VKGraphicsPipeline;
class RenderContext;
class CommandQueue;
class Surface;
class Fence;
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
    
//    void Execute() override;
    
    void Execute(RenderGraphNode node) override;
    
    VkDescriptorPool GetDescriptorPool() { return _descriptorPool; };
    
    VkCommandBuffer GetCommandBuffer() { return _commandBuffer; };
        
private:
    VkSemaphore _renderFinishedSemaphore;
    VkDescriptorPool _descriptorPool;
    VkCommandBuffer _commandBuffer;
    unsigned int _swapChainIndex;
    VkCommandPool _commandPool;
    VkFence _inFlightFence;
    std::shared_ptr<RenderContext> _device;
    
    static std::unordered_map<std::string, std::shared_ptr<VKGraphicsPipeline>> _pipelines; // Static, we want to be shared with other graphics context instances
};
