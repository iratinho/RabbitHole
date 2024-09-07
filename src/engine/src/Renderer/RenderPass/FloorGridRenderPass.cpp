#include "Renderer/RenderPass/FloorGridRenderPass.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphBuilder.hpp"
#include "Core/Scene.hpp"

GraphicsPipelineParams FloorGridRenderPass::GetPipelineParams() {
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;
    
    return pipelineParams;
}

RenderAttachments FloorGridRenderPass::GetRenderAttachments(GraphicsContext *graphicsContext) {
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

    return renderAttachments;
}

void FloorGridRenderPass::Process(RenderCommandEncoder *encoder, Scene* scene, GraphicsPipeline* pipeline) {
    using Components = std::tuple<PrimitiveProxyComponent, GridMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
    
    for(entt::entity entity : view) {
        BindPushConstants(encoder->GetGraphisContext(), pipeline, encoder, scene, entity);
        BindShaderResources(encoder->GetGraphisContext(), encoder, scene, entity);
        
        const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
        encoder->DrawPrimitiveIndexed(proxy);
    }
}

std::vector<ShaderResourceBinding> FloorGridRenderPass::CollectResourceBindings() {
    return {};
}

std::vector<PushConstant> FloorGridRenderPass::CollectPushConstants() { 
    std::vector<PushConstant> pushConstants;
    
    PushConstant pushConstant;
    
    constexpr PushConstantDataInfo<glm::mat4> infoMat4;
    pushConstant.name = "viewMatrix";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);

    pushConstant.name = "projMatrix";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);

    return pushConstants;
}

ShaderInputBindings FloorGridRenderPass::CollectShaderInputBindings() { 
    ShaderAttributeBinding vertexDataBinding;
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(VertexData);
    
    // Position vertex input
    ShaderInputLocation positions;
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);

    ShaderInputBindings inputBindings;
    inputBindings[vertexDataBinding] = {positions};
    return inputBindings;
}

void FloorGridRenderPass::BindPushConstants(GraphicsContext *graphicsContext, GraphicsPipeline *pipeline, RenderCommandEncoder *encoder, Scene *scene, EnttType entity) {
    
    Shader* vs = pipeline->GetVertexShader();
    if(!vs) {
        assert(0);
        return;
    }
    
    struct PushConstantData {
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
    } data;

    const auto cameraView = scene->GetRegistry().view<CameraComponent>();
    for(auto cameraEntity : cameraView) {
        // We should have a viewport abstraction that would know this type of information
        int width = graphicsContext->GetSwapChainColorTexture()->GetWidth();
        int height = graphicsContext->GetSwapChainColorTexture()->GetHeight();
        
        auto cameraComponent = cameraView.get<CameraComponent>(cameraEntity);
        data.viewMatrix = cameraComponent.m_ViewMatrix;
        data.projMatrix = glm::perspective(cameraComponent.m_Fov, ((float)width / (float)height), 0.1f, 180.f);
        break;
    }
    
    encoder->UpdatePushConstants(pipeline, vs, &data);
}

std::string FloorGridRenderPass::GetFragmentShaderPath() {
    return COMBINE_SHADER_DIR(floor_grid.frag);
}

std::string FloorGridRenderPass::GetVertexShaderPath() {
    return COMBINE_SHADER_DIR(floor_grid.vert);
}

std::set<std::shared_ptr<Texture2D>> FloorGridRenderPass::GetTextureResources(Scene* scene) {
    return {};
}
