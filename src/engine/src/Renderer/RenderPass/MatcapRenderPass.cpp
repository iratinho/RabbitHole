#include "Renderer/RenderPass/MatcapRenderPass.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphBuilder.hpp"
#include "Components/MatCapMaterialComponent.hpp"
#include "Core/Scene.hpp"

bool MatcapRenderPass::Setup(GraphBuilder* graphBuilder, GraphicsContext* graphicsContext, Scene* scene) {
    //    auto view = scene->GetRegistry().view<MatCapMaterial>();
    //
    //    // Discard this pass since there is nothing using it
    //    if(view.size() == 0) {
    //        return false;
    //    }
        
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;

    ColorAttachmentBlending blending;
    blending._colorBlending = BlendOperation::BLEND_OP_ADD;
    blending._alphaBlending = BlendOperation::BLEND_OP_ADD;
    blending._colorBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
    blending._alphaBlendingFactor = { BlendFactor::BLEND_FACTOR_ONE, BlendFactor::BLEND_FACTOR_ZERO };

    ColorAttachmentBinding colorAttachmentBinding;
    colorAttachmentBinding._texture = graphicsContext->GetSwapChainColorTexture();
    colorAttachmentBinding._blending = blending;
    colorAttachmentBinding._loadAction = LoadOp::OP_CLEAR;

    DepthStencilAttachmentBinding depthAttachmentBinding;
    depthAttachmentBinding._texture = graphicsContext->GetSwapChainDepthTexture();
    depthAttachmentBinding._depthLoadAction = LoadOp::OP_CLEAR;
    depthAttachmentBinding._stencilLoadAction = LoadOp::OP_DONT_CARE;
        
    RenderAttachments renderAttachments;
    renderAttachments._colorAttachmentBinding = colorAttachmentBinding;
    renderAttachments._depthStencilAttachmentBinding = depthAttachmentBinding;

    auto render = [this, scene, graphicsContext](class RenderCommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        MeshProcessor::Draw<MatCapMaterialComponent>(graphicsContext->GetDevice(), graphicsContext, scene, encoder, pipeline);
    };

    graphBuilder->AddRasterPass<MatCapMaterialComponent>("MatCapPass", scene, pipelineParams, renderAttachments, render);

    return true;
}
