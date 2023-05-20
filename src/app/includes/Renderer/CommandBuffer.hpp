#pragma once
#include "CommandPool.hpp"

class CommandPool;
class RenderContext;

enum ECommandBufferType {
    CBT_Default = 0
};

class CommandBuffer
{
public:
    CommandBuffer(RenderContext* renderContext, CommandPool* commandPool);

    void AllocateCommandBuffer();
    void EnableCommandBufferRecording() const;
    void DisableCommandBufferRecording() const;
    void ReleaseCommandBuffer();
    void* GetResource();

private:
    RenderContext* _renderContext;
    void* _commandBuffer;
    CommandPool* _commandPool;
};
