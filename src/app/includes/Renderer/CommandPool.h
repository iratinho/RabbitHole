#pragma once
#include "RenderPass/RenderPass.h"

class CommandPool : public ICommandPool {
public:
    CommandPool(RenderContext* renderContext, VkCommandPool commandPool);
    void ResetCommandPool() override;

private:
    VkCommandPool m_commandPool;
    RenderContext* m_renderContext;
};