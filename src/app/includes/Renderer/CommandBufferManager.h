#pragma once
#include "Renderer/render_context.h"

struct CommandBufferData
{
    int index = 0;
    bool bInUse = false;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
};

class CommandBufferManager
{
public:
    CommandBufferManager(RenderContext* renderContext);
    
    bool AllocateCommandBuffer(int frameIndex);

    bool ReleaseCommandBuffer(int frameIndex);

    VkCommandBuffer GetCommandBuffer(int frameIndex);
    
    VkCommandPool GetCommandBufferPool(VkCommandBuffer commandBuffer);
    
    VkCommandBuffer GetActiveCommandBuffer();
private:
    std::vector<CommandBufferData> m_CommandBuffers;
    RenderContext* m_RenderContext;
};
