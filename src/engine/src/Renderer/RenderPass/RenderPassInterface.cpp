#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphBuilder.hpp"
#include "Core/Scene.hpp"

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
        
    std::tie(params._vsParams, params._fsParams) = MakeShaderSet(this);
    
    _pipeline = GraphicsPipeline::Create(params);
    _pipeline->Compile();
    
    _graphicsContext = graphicsContext;
}

void RenderPass::EnqueueRendering(GraphBuilder* graphBuilder, Scene* scene) {
    auto RenderFunc = [this, scene](Encoders encoders, class GraphicsPipeline* pipeline) {
        Process(_graphicsContext, encoders, scene, pipeline);
    };
    
    graphBuilder->AddRasterPass(scene, this, RenderFunc);
}
