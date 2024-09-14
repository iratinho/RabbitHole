#include "Renderer/RenderPass/MatcapRenderPass.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphBuilder.hpp"
#include "Components/MatCapMaterialComponent.hpp"
#include "Core/Scene.hpp"

RenderAttachments MatcapRenderPass::GetRenderAttachments(GraphicsContext* graphicsContext) {
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

GraphicsPipelineParams MatcapRenderPass::GetPipelineParams() {
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;

    return pipelineParams;
}

ShaderInputBindings MatcapRenderPass::CollectShaderInputBindings() {
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

std::vector<ShaderResourceBinding> MatcapRenderPass::CollectResourceBindings() {    
    ShaderResourceBinding matCapTexSampler2D;
    matCapTexSampler2D._id = 0;
    matCapTexSampler2D._type = ShaderInputType::TEXTURE;
    matCapTexSampler2D._shaderStage = ShaderStage::STAGE_FRAGMENT;
    matCapTexSampler2D._identifier = "matCapTexture";

    std::vector<ShaderResourceBinding> resourceBindings;
    resourceBindings.push_back(matCapTexSampler2D);
    
    return resourceBindings;
}

std::vector<PushConstant> MatcapRenderPass::CollectPushConstants() {
    std::vector<PushConstant> pushConstants;
    
    PushConstant pushConstant;
    
    constexpr PushConstantDataInfo<glm::mat4> infoMat4;
    pushConstant.name = "mvp_matrix";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    pushConstant.name = "modelMatrix";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);

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

    pushConstant.name = "normalMatrix";
    pushConstant._dataType = infoMat4._dataType;
    pushConstant._size = infoMat4._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    constexpr PushConstantDataInfo<glm::mat4> infoVec3;
    pushConstant.name = "eyePosition";
    pushConstant._dataType = infoVec3._dataType;
    pushConstant._size = infoVec3._gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_FRAGMENT;
    pushConstants.push_back(pushConstant);

    return pushConstants;
}

void MatcapRenderPass::BindPushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* pipeline, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) {
    
    // Should we create a shader effect to handle mutable data? This shader effect would have runtime data and are backed by the shader, this way we could do Shader::MakeShaderEffect(shader); and then update the runtime data, we know
    // that this effect will share the same data as we declared it in the shader
    // The encoder would be encoder->BindShaderEffect but this could happen outside this classes or
    // we could call the shaderEffect->BindData(encoder), inside i would call
    // encoder->UpdatePushConstants and encoder->BindShaderResources
    Shader* fs = _pipeline->GetFragmentShader();
    if(!fs) {
        assert(0);
        return;
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
                    
        encoder->UpdatePushConstants(pipeline, fs, &transformComponent.m_Position);

        break;
    }
    
    auto view = scene->GetRegistry().view<TransformComponent>();
    const auto& transform = view.get<TransformComponent>(entity);
    
    glm::mat4 mvp = projMatrix * viewMatrix * transform._computedMatrix.value();
    
    struct PushConstants {
        glm::mat4 mvp_matrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
        glm::mat4 normalMatrix;
    };
    
    PushConstants vConstants;
    vConstants.mvp_matrix = mvp;
    vConstants.modelMatrix = transform._computedMatrix.value();
    vConstants.viewMatrix = viewMatrix;
    vConstants.projMatrix = projMatrix;
    vConstants.normalMatrix = glm::transpose(transform._computedMatrix.value());
    
    encoder->UpdatePushConstants(pipeline, _pipeline->GetVertexShader(), &vConstants);
}

void MatcapRenderPass::BindShaderResources(GraphicsContext* graphicsContext, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) {
    
    // Load textures from disk and collect all shader resources
    const auto view = scene->GetRegistry().view<MatCapMaterialComponent>();
    const auto& materialComponent = view.get<MatCapMaterialComponent>(entity);
    
//    materialComponent._matCapTexture->Initialize(graphicsContext->GetDevice());
//    
//    // If we already have a resource, it means that this texture is already in memory, should a GPU transfer here? Instead of the AddPass?? Confused
//    if(!materialComponent._matCapTexture->GetResource()) {
//        materialComponent._matCapTexture->Reload();
//    }
    
    if(!materialComponent._matCapTexture || (materialComponent._matCapTexture && !materialComponent._matCapTexture->GetResource())) {
        assert(0);
        return;
    }
    
    Shader* fs = _pipeline->GetFragmentShader();
    if(!fs) {
        assert(0);
        return;
    }
    
    ShaderTextureResource textureResource;
    textureResource._texture = materialComponent._matCapTexture;
    
    ShaderInputResource inputResource;
    inputResource._binding = fs->GetShaderResourceBinding("matCapTexture");
    inputResource._textureResource = textureResource;
    
    std::vector<ShaderInputResource> shaderResources;
    shaderResources.push_back(inputResource);
    
    encoder->BindShaderResources(fs, shaderResources);
}

std::string MatcapRenderPass::GetFragmentShaderPath() {
    return COMBINE_SHADER_DIR(matcap.frag);
}

std::string MatcapRenderPass::GetVertexShaderPath() {
    return COMBINE_SHADER_DIR(matcap.vert);
}

void MatcapRenderPass::Process(RenderCommandEncoder *encoder, Scene* scene, GraphicsPipeline* pipeline) {
    using Components = std::tuple<PrimitiveProxyComponent, MatCapMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
    
    for(entt::entity entity : view) {
        BindPushConstants(encoder->GetGraphisContext(), pipeline, encoder, scene, entity);
        BindShaderResources(encoder->GetGraphisContext(), encoder, scene, entity);
        
        const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
        encoder->DrawPrimitiveIndexed(proxy);
    }
}

// This should actually be the resources that we are writing too, like attachments and not this textures i think
std::set<std::shared_ptr<Texture2D>> MatcapRenderPass::GetTextureResources(Scene* scene) {
    std::set<std::shared_ptr<Texture2D>> textures;
    auto view = scene->GetRegistry().view<MatCapMaterialComponent>();
    for(auto entity : view) {
        auto materialComponent = view.get<MatCapMaterialComponent>(entity);
        textures.insert(materialComponent._matCapTexture);
    }
    return textures;
}
