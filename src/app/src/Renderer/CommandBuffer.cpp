#include "Renderer/CommandBuffer.hpp"
#include "Renderer/CommandPool.hpp"

CommandBuffer::CommandBuffer(RenderContext* renderContext, CommandPool* commandPool)
    : _renderContext(renderContext)
    , _commandBuffer(nullptr)
    , _commandPool(commandPool)
{
}

void CommandBuffer::AllocateCommandBuffer() {
    if(_renderContext && _commandPool) {
        _commandBuffer = static_cast<void*>(_renderContext->CreateCommandBuffer(_commandPool->GetResource()));
    }
}

void CommandBuffer::EnableCommandBufferRecording() const
{
    if(_renderContext && _commandPool) {
         _renderContext->BeginCommandBuffer(_commandBuffer);
    }
}

void CommandBuffer::DisableCommandBufferRecording() const
{
    if(_renderContext && _commandPool) {
        _renderContext->EndCommandBuffer(_commandBuffer);
    }
}

void CommandBuffer::ReleaseCommandBuffer()
{
    // TODO
}

void* CommandBuffer::GetResource() {
    return _commandBuffer;
}
