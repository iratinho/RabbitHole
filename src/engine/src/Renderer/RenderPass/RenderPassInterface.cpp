#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphicsPipeline.hpp"


void RenderPass::Initialize(GraphicsContext* graphicsContext) {
    GraphicsPipelineParams params = GetPipelineParams();
    params._shaderParams._renderAttachments = GetRenderAttachments(graphicsContext);
    params._shaderParams._shaderInputBindings = CollectShaderInputBindings();
    params._shaderParams._shaderInputConstants = CollectPushConstants();
    params._shaderParams._shaderResourceBindings = CollectResourceBindings();
    params._shaderParams._vsPath = GetVertexShaderPath();
    params._shaderParams._fsPath = GetFragmentShaderPath();
    params._device = graphicsContext->GetDevice();

    _pipeline = GraphicsPipeline::Create(params);
    _pipeline->Compile();    
}
