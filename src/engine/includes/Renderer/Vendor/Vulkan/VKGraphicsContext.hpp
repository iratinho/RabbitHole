#pragma once
#include "Renderer/GraphicsContext.hpp"
#include "vulkan/vulkan_core.h"

class VKGraphicsPipeline;
class Device;
class CommandQueue;
class Surface;
class Fence;
class Event;
class CommandBuffer;
class Texture2D;
class VKDescriptorManager;
class VKSamplerManager;

class VKGraphicsContext : public GraphicsContext {
public:
    VKGraphicsContext(Device* device);
    ~VKGraphicsContext();
    
    bool Initialize() override;
    
    void BeginFrame() override;
    
    void EndFrame() override;
    
    void Present() override;
        
    std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> GetPipelines() override;
    
    std::shared_ptr<Texture2D> GetSwapChainColorTexture() override;
    
    std::shared_ptr<Texture2D> GetSwapChainDepthTexture() override;
    
    void Execute(RenderGraphNode node) override;
    
    VkDescriptorPool GetDescriptorPool() { return _descriptorPool; };
    
    VKDescriptorManager* GetDescriptorManager() { return _descriptorsManager.get(); };
    
    VKSamplerManager* GetSamplerManager() { return _samplerManager.get(); };
            
private:
    VkDescriptorPool _descriptorPool;
    unsigned int _swapChainIndex;
    
    static std::unordered_map<std::string, std::shared_ptr<VKGraphicsPipeline>> _pipelines; // Static, we want to be shared with other graphics context instances
    
    std::shared_ptr<Fence> _fence;
    std::shared_ptr<Event> _event;
    std::shared_ptr<CommandBuffer> _commandBuffer;
    std::unique_ptr<VKDescriptorManager> _descriptorsManager;

    // Samplers are read-only they can be shared between graphics context
    static std::unique_ptr<VKSamplerManager> _samplerManager;
};
