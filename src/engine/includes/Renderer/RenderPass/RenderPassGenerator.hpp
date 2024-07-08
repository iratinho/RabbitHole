#pragma once
#include "Renderer/GPUDefinitions.h"
#include "Renderer/RenderGraph/RenderGraph.hpp"

class RenderTarget;
class RenderContext;
class Buffer;
class FrameResources;

class RenderPassGenerator {
public:
    RenderPassGenerator();
    ~RenderPassGenerator();

    RasterizationConfiguration& ConfigureRasterizationOptions();
    ShaderConfiguration& ConfigureShader(ShaderStage shaderStage);
    AttachmentConfiguration& MakeAttachment();
    PushConstantConfiguration* MakePushConstant();
    ShaderInputGroup& MakeShaderInputGroup();
    ShaderInput& MakeShaderInput(ShaderInputGroup& shaderInputGroup, unsigned int offset);
    PrimitiveProxy& MakePrimitiveProxy();
    void MakeTextureSampler(std::shared_ptr<RenderTarget> renderTarget, Sampler sampler);
    void AddPrimitiveProxy(PrimitiveProxy&& primitiveProxy);
private:
    PipelineStateObject* Generate(RenderContext* renderContext, unsigned int frameIndex, FrameResources* frameResources);

    RasterizationConfiguration _rasterizationConfiguration;
    std::vector<AttachmentConfiguration> _attachments;
    std::vector<PushConstantConfiguration> _pushConstants;
    std::vector<ShaderInputGroup> _shaderInputGroup;
    std::vector<PrimitiveProxy> _primitiveData;
    std::vector<TextureSampler> _textureSamplers; // textures to be sampled in the shaders
    std::unordered_map<ShaderStage, ShaderConfiguration> _shaderConfiguration;

    friend class RenderPassExecutor;
};
