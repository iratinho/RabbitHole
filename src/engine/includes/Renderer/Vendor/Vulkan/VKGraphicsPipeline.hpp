#pragma once
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GPUDefinitions.h"
#include "vulkan/vulkan_core.h"

class GraphicsContext;
class Texture2D;

class VKGraphicsPipeline : public GraphicsPipeline {
public:
    using VertexStateData = std::pair<std::vector<VkVertexInputBindingDescription>, std::vector<VkVertexInputAttributeDescription>>;

public:
    VKGraphicsPipeline(const GraphicsPipelineParams& params)
        : GraphicsPipeline(params) {
    };
                    
    void Compile() override;
    
    VkFramebuffer CreateFrameBuffer(std::vector<Texture2D*> textures);
    
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
    std::vector<VkPipelineColorBlendAttachmentState>  CreateColorBlendAttachemnt();
    
    std::vector<VkAttachmentDescription> CreateAttachmentDescriptions();
    
    std::vector<VkAttachmentReference> CreateColorAttachmentRef();
    
    VkAttachmentReference CreateDepthAttachmentRef();
    
    VkRenderPass CreateRenderPass();
    
    VertexStateData BuildVertexStateData();
    VkPushConstantRange BuildPushConstants();
    std::vector<VkDescriptorSetLayout> BuildDescriptorSetLayouts();
    
    
private:
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    std::unordered_map<uint32_t, VkFramebuffer> _frameBuffers;
    std::vector<VkImageView> _views;
    VkRenderPass _renderPass;
    VkPipeline _pipeline;
    VkPipelineLayout _pipelineLayout;
    bool _bWasCompiled = false;
};
