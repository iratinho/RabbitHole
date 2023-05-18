#include "Renderer/CommandPool.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/Fence.h"
#include "Renderer/Swapchain.h"

CommandPool::CommandPool(RenderContext* renderContext, VkCommandPool commandPool)
    : m_commandPool(commandPool)
    , m_renderContext(renderContext)
{
}

CommandPool::CommandPool(RenderContext* renderContext)
    : m_renderContext(renderContext)
{
}

void CommandPool::AllocateCommandPool() {
    if(m_renderContext && m_commandPool == VK_NULL_HANDLE) {
        m_commandPool = m_renderContext->CreateCommandPool();
    }
}

void CommandPool::AllocateCommandBuffer()
{
    if(!_commandBuffer) {
        _commandBuffer = std::make_shared<CommandBuffer>(m_renderContext, this);
        _commandBuffer->AllocateCommandBuffer();
    }
}

void CommandPool::EnableCommandBufferRecording()
{
    if(_commandBuffer) {
        _commandBuffer->EnableCommandBufferRecording();
    }
}

void CommandPool::DisableCommandBufferRecording()
{
    if(_commandBuffer) {
        _commandBuffer->DisableCommandBufferRecording();
    }
}

void CommandPool::ReleaseCommandBuffer()
{
    // TODO
}

void CommandPool::ResetCommandPool() {
    m_renderContext->ResetCommandPool(m_commandPool);
}

void* CommandPool::GetResource() {
    return m_commandPool;
}

void CommandPool::SubmitCommands(const SubmitCommandParams& submitParams)
{
    const auto commandBuffer = static_cast<VkCommandBuffer>(_commandBuffer->GetResource());
    const auto waitSemaphores = static_cast<VkSemaphore>(submitParams.waitSemaphore);
    const auto signalSemaphores = static_cast<VkSemaphore>(submitParams.signalSemaphore);
    const VkFence fence = static_cast<VkFence>(submitParams.queueFence->GetResource());
    
    VkSubmitInfo submit_info{};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pNext = nullptr;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer;
    submit_info.pWaitSemaphores = &waitSemaphores;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = waitStages;
    submit_info.pSignalSemaphores = &signalSemaphores;
    submit_info.signalSemaphoreCount = 1;
    VkFunc::vkQueueSubmit(m_renderContext->GetGraphicsQueueHandle(), 1, &submit_info, fence);
}

std::shared_ptr<CommandBuffer> CommandPool::GetCommandBuffer() {
    return _commandBuffer;
}
