#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphicsPipeline.hpp"

namespace {
    std::pair<ShaderParams, ShaderParams> MakeShaderSet(RenderPass* renderPass) {
        if(!renderPass) {
            return {};
        }
                
        const auto& dataStreams = renderPass->CollectShaderDataStreams();

        ShaderParams vsParams;
        vsParams._shaderPath = renderPass->GetVertexShaderPath();
        vsParams._shaderInputBindings = renderPass->CollectShaderInputBindings();
        vsParams._shaderDataStreams = dataStreams;
        
        ShaderParams fsParams;
        fsParams._shaderPath = renderPass->GetFragmentShaderPath();
        fsParams._shaderDataStreams = dataStreams;
                
        return {vsParams, fsParams};
    }
}

void RenderPass::Initialize(GraphicsContext* graphicsContext) {
    GraphicsPipelineParams params = GetPipelineParams();
    params._renderAttachments = GetRenderAttachments(graphicsContext);
    params._device = graphicsContext->GetDevice();
    params._renderPass = this;
    
    // TODO create a ShaderWrapper class that contains the shaders + genric data shared from the shaders
    // We can also instanciate the shaders from the wrapper
    
    std::tie(params._vsParams, params._fsParams) = MakeShaderSet(this);
    
    _pipeline = GraphicsPipeline::Create(params);
    _pipeline->Compile();    
    
    _graphicsContext = graphicsContext;
}
