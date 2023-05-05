#include "Renderer/CommandBufferManager.h"

CommandBufferManager::CommandBufferManager(RenderContext* renderContext)
    : m_RenderContext(renderContext)
{
}

bool CommandBufferManager::AllocateCommandBuffer(int frameIndex) {
    // For now we are creating command pools with command buffer for easy tracking but once we have a frame abstraction
    // the frame class should be the one to create the command pool and then the manager would have a map<commandPool, vec<commandBuffer>>
    if(m_RenderContext) {
        // If we already have a command buffer for this frame index lets make it active
        const auto entry = std::ranges::find_if(m_CommandBuffers, [&](CommandBufferData& cmdData){ return cmdData.index == frameIndex; });

        // We found an entry
        if(entry != m_CommandBuffers.end()) {
            std::ranges::for_each(m_CommandBuffers, [&](CommandBufferData& cmdData) {
                if(cmdData.index == frameIndex) {
                    cmdData.bInUse = true;
                }
                
            });
            return true;
        }
        
        if(const VkCommandPool commandPool = m_RenderContext->CreateCommandPool()) {
            if(const VkCommandBuffer commandBuffer = m_RenderContext->CreateCommandBuffer(commandPool)) {
                CommandBufferData cmdBuffer;
                cmdBuffer.commandBuffer = commandBuffer;
                cmdBuffer.commandPool = commandPool;
                cmdBuffer.index = frameIndex;
                cmdBuffer.bInUse = true;
                
                m_CommandBuffers.push_back(cmdBuffer);
                return true;
            }        
        }
    }
    
    return false;
}

bool CommandBufferManager::ReleaseCommandBuffer(int frameIndex) {
    if(m_RenderContext) {
        const auto entry = std::ranges::find_if(m_CommandBuffers, [&](CommandBufferData& cmdData){ return cmdData.index == frameIndex; });

        if(entry != m_CommandBuffers.end()) {
            std::ranges::for_each(m_CommandBuffers, [&](CommandBufferData& cmdData) {
                if(cmdData.index == frameIndex) {
                    cmdData.bInUse = false;    
                }
            });
            return true;
        }
    }

    return false;
}

VkCommandBuffer CommandBufferManager::GetCommandBuffer(int frameIndex) {
    const auto entry = std::ranges::find_if(m_CommandBuffers, [frameIndex](CommandBufferData& cmdData){ return cmdData.index == frameIndex; });
    if(entry != m_CommandBuffers.end()) {
        return entry->commandBuffer;
    }

    return VK_NULL_HANDLE;
}

VkCommandPool CommandBufferManager::GetCommandBufferPool(VkCommandBuffer commandBuffer) {
    auto entry = std::ranges::find_if(m_CommandBuffers, [&](CommandBufferData& cmdData) { return cmdData.commandBuffer == commandBuffer; });
    if(entry != m_CommandBuffers.end()) {
        return entry->commandPool;
    }

    return VK_NULL_HANDLE;
}

VkCommandBuffer CommandBufferManager::GetActiveCommandBuffer() {
    const auto entry = std::ranges::find_if(m_CommandBuffers, [](CommandBufferData& cmdData){ return cmdData.bInUse; });
    if(entry != m_CommandBuffers.end()) {
        return entry->commandBuffer;
    }

    return VK_NULL_HANDLE;
}
