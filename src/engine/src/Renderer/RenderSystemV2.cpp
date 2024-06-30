#include "Components/DirectionalLightComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Components/MatCapMaterialComponent.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"
#include "Renderer/Processors/TransformProcessor.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/TransferContext.hpp"
#include "Renderer/RenderSystemV2.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Buffer.hpp"

#include "Core/MeshObject.hpp"
#include "Core/Camera.hpp"
#include "Core/Scene.hpp"
#include "Core/Utils.hpp"

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_DIR) "/" STR(name)

RenderSystemV2::RenderSystemV2() {
}

RenderSystemV2::~RenderSystemV2() {
}

bool RenderSystemV2::Initialize(const InitializationParams& params) {
    _device = std::make_shared<Device>(nullptr);
    if(!_device->Initialize(params)) {
        return false;
    }

    // Create a new graphics context per swapchain image
    constexpr unsigned int swapChainCount = 2;
    for (size_t i = 0; i < swapChainCount; i++) {
        auto context = GraphicsContext::Create(_device);
        if(!context->Initialize()) {
            return false;
        }
        
        _graphicsContext.push_back(context);
    }
    
    _transferContext = TransferContext::Create(_device.get());
        
    return true;
}

bool RenderSystemV2::Process(Scene* scene) {
    if(currentContext > 1) {
        currentContext = 0;
    }
    
    BeginFrame(scene);
    Render(scene);
    EndFrame();
    
    currentContext++;
    
    return true;
}

void RenderSystemV2::BeginFrame(Scene* scene) {
    auto graphicsContext = _graphicsContext[currentContext];
    if(!graphicsContext) {
        assert(0);
        return;
    }
    
    // Create a new graph builder per frame, this as no cost
    _graphBuilder = GraphBuilder(graphicsContext.get());
    
    // Once we are using unified buffers could we create this per draw? and sync the transfer queue at the end?
    // Upload to GPU side all geometry resources
    auto buffer = MeshProcessor::GenerateBuffer(_device.get(), scene);
     if(buffer) {
         _transferContext->EnqueueBufferSync(buffer);
         _transferContext->Flush();
     }

    // Updates all transforms in the scene to be used when rendering
    TransformProcessor::Process(scene);
    
    graphicsContext->BeginFrame();
}

void RenderSystemV2::Render(Scene* scene) {
    auto graphicsContext = _graphicsContext[currentContext].get();
    if(!graphicsContext) {
        return false;
    }
    
    SetupMatCapRenderPass(graphicsContext, scene);
    SetupBasePass(graphicsContext, scene);
    SetupFloorGridRenderPass(graphicsContext, scene);
        
    // Execute all rendering commands
    _graphBuilder.Exectue([this, graphicsContext](RenderGraphNode node) {
        graphicsContext->Execute(node);
    });

}

void RenderSystemV2::EndFrame() {
    auto& graphicsContext = _graphicsContext[currentContext];
    if(!graphicsContext) {
        assert(0);
        return;
    }
    
    graphicsContext->EndFrame();
    graphicsContext->Present();
}

bool RenderSystemV2::SetupMatCapRenderPass(GraphicsContext* graphicsContext, Scene* scene) {
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

    auto render = [this, scene, graphicsContext](class CommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        MeshProcessor::Draw<MatCapMaterialComponent>(_device.get(), graphicsContext, scene, encoder, pipeline);
    };

    _graphBuilder.AddRasterPass<MatCapMaterialComponent>("MatCapPass", pipelineParams, renderAttachments, render);

    return true;
}

bool RenderSystemV2::SetupBasePass(GraphicsContext* graphicsContext, Scene* scene) {
    auto view = scene->GetRegistry().view<PhongMaterialComponent>();
    
    // Discard this pass since there is nothing using it
    if(view.size() == 0) {
        return false;
    }
        
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
        
    auto render = [this, scene, graphicsContext](class CommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        MeshProcessor::Draw<PhongMaterialComponent>(_device.get(), graphicsContext, scene, encoder, pipeline);
    };
    
    _graphBuilder.AddRasterPass<PhongMaterialComponent>("BasePass", pipelineParams, renderAttachments, render);
    
    return true;
}

bool RenderSystemV2::SetupFloorGridRenderPass(GraphicsContext* graphicsContext, Scene* scene) {
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
    colorAttachmentBinding._loadAction = LoadOp::OP_LOAD;
    
    DepthStencilAttachmentBinding depthAttachmentBinding;
    depthAttachmentBinding._texture = graphicsContext->GetSwapChainDepthTexture();
    depthAttachmentBinding._depthLoadAction = LoadOp::OP_LOAD;
    depthAttachmentBinding._stencilLoadAction = LoadOp::OP_DONT_CARE;
    
    RenderAttachments renderAttachments;
    renderAttachments._colorAttachmentBinding = colorAttachmentBinding;
    renderAttachments._depthStencilAttachmentBinding = depthAttachmentBinding;
    
    auto render = [this, scene, graphicsContext](class CommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        MeshProcessor::Draw<GridMaterialComponent>(_device.get(), graphicsContext, scene, encoder, pipeline);        
    };
        
    _graphBuilder.AddRasterPass<GridMaterialComponent>("FloorPass", pipelineParams, renderAttachments, render);

    return true;
}

// Need to stop using swapchain render targets all the time, it makes sync complicated, instead lets have pass textures that wen can more easy use them
