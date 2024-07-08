#pragma once
#include "Renderer/RenderPass.hpp"

class VkGraphicsPipeline;

class VKRenderPass : public RenderPass {
public:
    VKRenderPass(std::shared_ptr<VkGraphicsPipeline> pipeline);
    
    void BeginRenderPass() override;
    
    void EndRenderPass() override;
    
private:
    std::shared_ptr<VkGraphicsPipeline> _pipeline;
};
