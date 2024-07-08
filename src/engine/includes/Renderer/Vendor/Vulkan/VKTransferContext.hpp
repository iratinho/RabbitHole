#pragma once
#include "Renderer/TransferContext.hpp"
#include "vulkan/vulkan_core.h"

class VKTransferContext : public TransferContext {
public:
    
    void EnqueueBufferSync(std::shared_ptr<Buffer> buffer) override;

    void Flush() override;
  
private:
    void InitializeResources();
    
private:
    VkCommandPool _commandPool = VK_NULL_HANDLE;
    VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
    VkFence _fence = VK_NULL_HANDLE;
    bool _isPending = false;
    
    // Mostly used to increase ref count for buffers so we dont even try to delete while in transition
    std::vector<std::shared_ptr<Buffer>> _bufferList;
};
