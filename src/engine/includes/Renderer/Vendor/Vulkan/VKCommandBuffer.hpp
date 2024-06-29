#pragma once
#include "Renderer/CommandBuffer.hpp"
#include "vulkan/vulkan_core.h"

class VKCommandBuffer : public CommandBuffer {
public:
    void BeginRecording() override;
    void EndRecording() override;
    void Submit(std::shared_ptr<Fence> fence = nullptr) override;
    void Present(uint32_t swapChainIndex) override;
    
public:
    VkCommandBuffer GetVkCommandBuffer() {
        return _commandBuffer;
    }
    
protected:
    bool Initialize() override;
    
private:
    static VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    VkFence _inFlightFence;
    
    // This should be unique
    std::shared_ptr<Event>  _event;
};
