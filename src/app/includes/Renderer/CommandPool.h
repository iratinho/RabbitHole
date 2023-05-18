#pragma once
#include "RenderPass/RenderPass.h"

class CommandBuffer;
class Fence;

struct SubmitCommandParams
{
    // Commands will only be submitted when this semaphore is released
    void* waitSemaphore;

    // This semaphore will be in signaled when the commands have been submitted
    void* signalSemaphore;

    // Fence used to allow us to wait for queue execution completion 
    Fence* queueFence;
};

class ICommandPool {
public:
    virtual ~ICommandPool()
    {
        
    };

    virtual void AllocateCommandPool() = 0;
    virtual void AllocateCommandBuffer() = 0;
    virtual void EnableCommandBufferRecording() = 0;
    virtual void DisableCommandBufferRecording() = 0;
    virtual void ReleaseCommandBuffer() = 0;
    virtual void ResetCommandPool() = 0;
    virtual void* GetResource() = 0;
    virtual void SubmitCommands(const SubmitCommandParams& submitParams) = 0;
};

class CommandPool : public ICommandPool {
public:

    CommandPool(RenderContext* renderContext, VkCommandPool commandPool);
    CommandPool(RenderContext* renderContext);

    void AllocateCommandPool() override;
    void AllocateCommandBuffer() override;
    void EnableCommandBufferRecording() override;
    void DisableCommandBufferRecording() override;
    void ReleaseCommandBuffer() override;
    void ResetCommandPool() override;
    void* GetResource() override;
    void SubmitCommands(const SubmitCommandParams& submitParams) override;

    std::shared_ptr<CommandBuffer> GetCommandBuffer();
    
private:
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    RenderContext* m_renderContext;
        
    std::shared_ptr<CommandBuffer> _commandBuffer;
};
