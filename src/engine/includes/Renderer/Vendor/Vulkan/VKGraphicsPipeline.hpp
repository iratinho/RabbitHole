#pragma once
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GPUDefinitions.h"
#include "vulkan/vulkan_core.h"

class GraphicsContext;

class VKGraphicsPipeline : public GraphicsPipeline {
public:
    VKGraphicsPipeline(const GraphicsPipelineParams& params)
        : GraphicsPipeline(params) {
    };
                    
    void Compile() override;
    
    void Draw(const PrimitiveProxy& proxy) override;

    VkFramebuffer CreateFrameBuffer(std::vector<RenderTarget*> renderTargets);
    
    void DestroyFrameBuffer();
    
    VkRenderPass GetVKPass() {
        return _renderPass;
    }

    VkPipeline GetVKPipeline() {
        return _pipeline;
    }
    
    VkPipelineLayout GetVKPipelineLayout() {
        return _pipelineLayout;
    }
    
private:
    VkResult CreateDescriptorsSets(std::vector<VkDescriptorSetLayout>&  descriptorLayouts);
        
    std::vector<VkPipelineColorBlendAttachmentState>  CreateColorBlendAttachemnt();
    
    std::vector<VkAttachmentDescription> CreateAttachmentDescriptions();
    
    std::vector<VkAttachmentReference> CreateColorAttachmentRef();
    
    VkAttachmentReference CreateDepthAttachmentRef();
    
    VkRenderPass CreateRenderPass();
    
private:
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    VkFramebuffer _frameBuffer = VK_NULL_HANDLE;
    std::vector<VkImageView> _views;
    VkRenderPass _renderPass;
    VkPipeline _pipeline;
    VkPipelineLayout _pipelineLayout;
    bool _bWasCompiled = false;
};
