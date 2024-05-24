#include "Components/DirectionalLightComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/GridMaterialComponent.hpp"

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
    
    // Setup Render passes
    auto& graphicsContext = _graphicsContext[0];
    
//    auto range = Range(0, 10);
//    std::for_each(range.begin(), range.end(), [](unsigned int value) {
//        std::cout << value << std::endl;
//    });
    
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
    
    SetupBasePass(graphicsContext, scene);
    SetupFloorGridRenderPass(graphicsContext, scene);
    
    // Execute all rendering commands
    graphicsContext->Flush();
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

// How to improve locality in this situation
void RenderSystemV2::ProcessGeometry(Scene *scene) {
}

bool RenderSystemV2::SetupMatCapRenderPass(GraphicsContext* graphicsContext) {
//    // TODO add support for identifier for pipeline
//    std::shared_ptr<GraphicsPipeline> pipeline = graphicsContext->AllocatePipeline("MapcapPass");
//    if(!pipeline) {
//        return;
//    }
//
//    pipeline->SetShader(ShaderStage::STAGE_VERTEX, COMBINE_SHADER_DIR(matcap.vert));
//    pipeline->SetShader(ShaderStage::STAGE_FRAGMENT, COMBINE_SHADER_DIR(matcap.frag));
//
//    // Push constants
//    pipeline->DeclarePushConstant<glm::vec3>(ShaderStage::STAGE_VERTEX, "eye");
//    pipeline->DeclarePushConstant<glm::mat4>(ShaderStage::STAGE_VERTEX, "mvp");
//
//    // Binding 0 for vertex data
//    ShaderInputBinding vertexDataBinding;
//    vertexDataBinding._binding = 0;
//    vertexDataBinding._stride = sizeof(VertexData);
//
//    // Position vertex input
//    ShaderInputLocation positions;
//    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
//    positions._offset = offsetof(VertexData, position);
//    pipeline->SetVertexInput(vertexDataBinding, positions);
//
//    // Normals vertex input
//    ShaderInputLocation normals;
//    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
//    positions._offset = offsetof(VertexData, normal);
//    pipeline->SetVertexInput(vertexDataBinding, normals);
//
//    // 2D Sampler
////    pipeline->DeclareSampler(scene->GetMatCapMaterial()->GetMatCapTexture());
//    
//    pipeline->Compile();
}

bool RenderSystemV2::SetupBasePass(GraphicsContext* graphicsContext, Scene* scene) {
    MaterialProcessor<PhongMaterialComponent>::GenerateShaders(graphicsContext);
    
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;
    pipelineParams._vertexShader = MaterialProcessor<PhongMaterialComponent>::GetVertexShader(graphicsContext);
    pipelineParams._fragmentShader = MaterialProcessor<PhongMaterialComponent>::GetFragmentShader(graphicsContext);
    pipelineParams._id = currentContext;
    
    ColorAttachmentBlending blending;
    blending._colorBlending = BlendOperation::BLEND_OP_ADD;
    blending._alphaBlending = BlendOperation::BLEND_OP_ADD;
    blending._colorBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
    blending._alphaBlendingFactor = { BlendFactor::BLEND_FACTOR_ONE, BlendFactor::BLEND_FACTOR_ZERO };

    ColorAttachmentBinding colorAttachmentBinding;
    colorAttachmentBinding._renderTarget = graphicsContext->GetSwapchainColorTarget();
    colorAttachmentBinding._blending = blending;
    colorAttachmentBinding._loadAction = LoadOp::OP_CLEAR;

    DepthStencilAttachmentBinding depthAttachmentBinding;
    depthAttachmentBinding._renderTarget = graphicsContext->GetSwapchainDepthTarget();
    depthAttachmentBinding._depthLoadAction = LoadOp::OP_CLEAR;
    depthAttachmentBinding._stencilLoadAction = LoadOp::OP_DONT_CARE;
    
    RenderAttachments renderAttachments;
    renderAttachments._colorAttachmentBinding = colorAttachmentBinding;
    renderAttachments._depthStencilAttachmentBinding = depthAttachmentBinding;
    
    auto render = [this, scene, graphicsContext](class CommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        MeshProcessor::Draw<PhongMaterialComponent>(_device.get(), graphicsContext, scene, encoder, pipeline);
    };
    
    graphicsContext->GetGraphBuilder().AddRasterPass("BasePass", pipelineParams, renderAttachments, render);
    
    return true;
}

bool RenderSystemV2::SetupFloorGridRenderPass(GraphicsContext* graphicsContext, Scene* scene) {
    MaterialProcessor<GridMaterialComponent>::GenerateShaders(graphicsContext);
        
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;
    pipelineParams._vertexShader = MaterialProcessor<GridMaterialComponent>::GetVertexShader(graphicsContext);
    pipelineParams._fragmentShader = MaterialProcessor<GridMaterialComponent>::GetFragmentShader(graphicsContext);
    pipelineParams._id = currentContext;
    
    ColorAttachmentBlending blending;
    blending._colorBlending = BlendOperation::BLEND_OP_ADD;
    blending._alphaBlending = BlendOperation::BLEND_OP_ADD;
    blending._colorBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
    blending._alphaBlendingFactor = { BlendFactor::BLEND_FACTOR_ONE, BlendFactor::BLEND_FACTOR_ZERO };
    
    ColorAttachmentBinding colorAttachmentBinding;
    colorAttachmentBinding._renderTarget = graphicsContext->GetSwapchainColorTarget();
    colorAttachmentBinding._blending = blending;
    colorAttachmentBinding._loadAction = LoadOp::OP_LOAD;
    
    DepthStencilAttachmentBinding depthAttachmentBinding;
    depthAttachmentBinding._renderTarget = graphicsContext->GetSwapchainDepthTarget();
    depthAttachmentBinding._depthLoadAction = LoadOp::OP_LOAD;
    depthAttachmentBinding._stencilLoadAction = LoadOp::OP_DONT_CARE;
    
    RenderAttachments renderAttachments;
    renderAttachments._colorAttachmentBinding = colorAttachmentBinding;
    renderAttachments._depthStencilAttachmentBinding = depthAttachmentBinding;
    
    auto render = [this, scene, graphicsContext](class CommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        MeshProcessor::Draw<GridMaterialComponent>(_device.get(), graphicsContext, scene, encoder, pipeline);        
    };
    
    graphicsContext->GetGraphBuilder().AddRasterPass("FloorPass", pipelineParams, renderAttachments, render);

    return true;
}
