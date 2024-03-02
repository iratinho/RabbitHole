#pragma once
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GPUDefinitions.h"
#include "vulkan/vulkan_core.h"

class GraphicsContext;

class VKGraphicsPipeline : public GraphicsPipeline {
public:
    using GraphicsPipeline::GraphicsPipeline;
    
//    void SetShader(ShaderStage shaderStage, std::unique_ptr<Shader>&& shader) override;
                
    void BeginVertexInput() override {};

    void SetVertexInputs(const std::vector<ShaderInputGroup> &inputGroup) override;
    
    void SetVertexInput(const ShaderInputBinding& binding, const ShaderInputLocation& location) override;
    
    void SetRasterizationParams(const RasterizationConfiguration &rasterizationConfiguration) override;

    void DeclareSampler(std::shared_ptr<Texture2D> textureSampler) override;

    void Compile() override;
    
    void Draw(const PrimitiveProxy& proxy) override;

    VkFramebuffer CreateFrameBuffer(std::vector<RenderTarget*> renderTargets);
    
    void DestroyFrameBuffer();
    
    [[nodiscard]] VkRenderPass GetVKPass() {
        return _renderPass;
    }

private:    
    VkResult CreateDescriptorsSets(std::vector<VkDescriptorSetLayout>&  descriptorLayouts);
        
    std::vector<VkPipelineColorBlendAttachmentState>  CreateColorBlendAttachemnt();
    
    std::vector<VkAttachmentDescription> CreateAttachmentDescriptions();
    
    std::vector<VkAttachmentReference> CreateColorAttachmentRef();
    
    VkAttachmentReference CreateDepthAttachmentRef();
    
    VkRenderPass CreateRenderPass();
        
    
private:
    RasterizationConfiguration _rasterizationConfiguration;
    std::map<ShaderInputBinding, std::vector<ShaderInputLocation>> _shaderInputs;
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    std::vector<std::shared_ptr<Texture2D>> _textureSamplers;
    VkFramebuffer _frameBuffer = VK_NULL_HANDLE;
    std::vector<VkImageView> _views;
    VkRenderPass _renderPass;
    bool _bWasCompiled = false;
};
