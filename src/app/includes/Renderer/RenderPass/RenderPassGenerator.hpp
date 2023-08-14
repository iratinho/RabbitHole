#pragma once
#include "Renderer/GPUDefinitions.h"
#include "Renderer/RenderGraph/RenderGraph.hpp"

class RenderTarget;
class RenderContext;
class Buffer;

class RenderPassGenerator {
public:
    RenderPassGenerator();

    RasterizationConfiguration& ConfigureRasterizationOptions();
    ShaderConfiguration& ConfigureShader(ShaderStage shaderStage);
    AttachmentConfiguration& MakeAttachment();
    PushConstantConfiguration& MakePushConstant();
    InputGroupDescriptor& MakeInputGroupDescriptor();
    PrimitiveProxy& MakePrimitiveProxy();
    void AddPrimitiveProxy(PrimitiveProxy&& primitiveProxy);
private:
    PipelineStateObject* Generate(RenderContext* renderContext, unsigned int frameIndex);

    RasterizationConfiguration _rasterizationConfiguration;
    std::vector<AttachmentConfiguration> _attachments;
    std::vector<PushConstantConfiguration> _pushConstants;
//    std::vector<InputDescriptor> _inputDescriptors;
    std::vector<InputGroupDescriptor> _inputGroupDescriptors;
    std::vector<PrimitiveProxy> _primitiveData;
    std::unordered_map<ShaderStage, ShaderConfiguration> _shaderConfiguration;

    friend class RenderPassExecutor;
};
