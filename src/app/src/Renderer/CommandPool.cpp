#include "Renderer/CommandPool.h"

CommandPool::CommandPool(RenderContext* renderContext, VkCommandPool commandPool)
    : m_commandPool(commandPool)
    , m_renderContext(renderContext)
{
}

void CommandPool::ResetCommandPool() {
    m_renderContext->ResetCommandPool(m_commandPool);
}
