
#include "Renderer/RenderPass/PhongRenderPass.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphBuilder.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Core/Scene.hpp"

RenderAttachments PhongRenderPass::GetRenderAttachments(GraphicsContext *graphicsContext) { 
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
    
    return renderAttachments;
}

GraphicsPipelineParams PhongRenderPass::GetPipelineParams() {
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;

    return pipelineParams;
}

void PhongRenderPass::BindPushConstants(GraphicsContext *graphicsContext, GraphicsPipeline *pipeline, RenderCommandEncoder *encoder, Scene *scene, EnttType entity) { 
    Shader* vertexShader = MaterialProcessor<PhongMaterialComponent>::GetVertexShader(graphicsContext).get();
    Shader* fragmentShader = MaterialProcessor<PhongMaterialComponent>::GetFragmentShader(graphicsContext).get();
    
    struct PushConstantData {
        glm::mat4 mvp;
        alignas(16) glm::vec3 _color;
        alignas(16) glm::vec3 _direction;
        alignas(16) float _intensity;
        alignas(16) glm::vec3 cameraPosition;
    } data;

    // Lights
    const auto directionalLightView = scene->GetRegistry().view<DirectionalLightComponent>();
    for (auto lightEntity : directionalLightView) {
        auto& lightComponent = directionalLightView.get<DirectionalLightComponent>(lightEntity);
        
        data._color = lightComponent._color;
        data._direction = lightComponent._direction;
        data._intensity = lightComponent._intensity;

        break;
    }
    
    // We should have a viewport abstraction that would know this type of information
    int width = graphicsContext->GetSwapChainColorTexture()->GetWidth();
    int height = graphicsContext->GetSwapChainColorTexture()->GetHeight();
    
    // Camera
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::vec3 cameraPosition;
    const auto cameraView = scene->GetRegistry().view<TransformComponent, CameraComponent>();
    for(auto cameraEntity : cameraView) {
        auto [transformComponent, cameraComponent] = cameraView.get<TransformComponent, CameraComponent>(cameraEntity);
        viewMatrix = cameraComponent.m_ViewMatrix;
        projMatrix = glm::perspective(cameraComponent.m_Fov, ((float)width / (float)height), 0.1f, 180.f);
        
        data.cameraPosition = transformComponent.m_Position;
        
        break;
    }
    
    // MVP matrix
    auto view = scene->GetRegistry().view<TransformComponent>();
    const auto& transform = view.get<TransformComponent>(entity);
    data.mvp = projMatrix * viewMatrix * transform._computedMatrix.value();

    encoder->UpdatePushConstants(pipeline, vertexShader, &data);
}

void PhongRenderPass::Process(RenderCommandEncoder *encoder, Scene* scene, GraphicsPipeline* pipeline) {
    using Components = std::tuple<PrimitiveProxyComponent, PhongMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
        
    for(entt::entity entity : view) {
        BindPushConstants(encoder->GetGraphisContext(), pipeline, encoder, scene, entity);
        BindShaderResources(encoder->GetGraphisContext(), encoder, scene, entity);
        
        const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
        encoder->DrawPrimitiveIndexed(proxy);
    }
}

ShaderInputBindings PhongRenderPass::CollectShaderInputBindings() {
    ShaderAttributeBinding vertexDataBinding;
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(VertexData);

    // Position vertex input
    ShaderInputLocation positions;
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);
    
    ShaderInputLocation normals;
    normals._format = Format::FORMAT_R32G32B32_SFLOAT;
    normals._offset = offsetof(VertexData, normal);

    ShaderInputBindings inputBindings;
    inputBindings[vertexDataBinding] = {positions, normals};
    return inputBindings;
}


std::vector<ShaderResourceBinding> PhongRenderPass::CollectResourceBindings() { 
    return {};
}

std::vector<PushConstant> PhongRenderPass::CollectPushConstants() {
    std::vector<PushConstant> pushConstants;
    
    PushConstant pushConstant;
    
    constexpr PushConstantDataInfo<glm::mat4> infoMat4;
    pushConstant.name = "mvp_matrix";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    pushConstant.name = "lightColor";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    pushConstant.name = "lightDirection";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    constexpr PushConstantDataInfo<float> infoFloat;
    pushConstant.name = "lightIntensity";
    pushConstant._dataType = infoFloat._dataType;
    pushConstant._size = infoFloat._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);

    constexpr PushConstantDataInfo<glm::mat4> infoVec3;
    pushConstant.name = "cameraPosition";
    pushConstant._dataType = infoVec3._dataType;
    pushConstant._size = infoVec3._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_FRAGMENT;
    pushConstants.push_back(pushConstant);

    return pushConstants;
}

std::string PhongRenderPass::GetFragmentShaderPath() { 
    return COMBINE_SHADER_DIR(dummy.frag);
}

std::string PhongRenderPass::GetVertexShaderPath() { 
    return COMBINE_SHADER_DIR(dummy.vert);
}

std::set<std::shared_ptr<Texture2D>> PhongRenderPass::GetTextureResources(Scene* scene) {
    return {};
}

