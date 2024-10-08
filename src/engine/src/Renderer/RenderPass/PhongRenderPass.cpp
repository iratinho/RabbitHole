
#include "Renderer/RenderPass/PhongRenderPass.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Core/Scene.hpp"

RenderAttachments PhongRenderPass::GetRenderAttachments(GraphicsContext *graphicsContext) {
    GraphicsPipelineParams pipelineParams = {};
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;
    
    ColorAttachmentBlending blending = {};
    blending._colorBlending = BlendOperation::BLEND_OP_ADD;
    blending._alphaBlending = BlendOperation::BLEND_OP_ADD;
    blending._colorBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
    blending._alphaBlendingFactor = { BlendFactor::BLEND_FACTOR_ONE, BlendFactor::BLEND_FACTOR_ZERO };

    ColorAttachmentBinding colorAttachmentBinding = {};
    colorAttachmentBinding._texture = graphicsContext->GetSwapChainColorTexture();
    colorAttachmentBinding._blending = blending;
    colorAttachmentBinding._loadAction = LoadOp::OP_CLEAR;

    DepthStencilAttachmentBinding depthAttachmentBinding = {};
    depthAttachmentBinding._texture = graphicsContext->GetSwapChainDepthTexture();
    depthAttachmentBinding._depthLoadAction = LoadOp::OP_CLEAR;
    depthAttachmentBinding._stencilLoadAction = LoadOp::OP_DONT_CARE;
    
    RenderAttachments renderAttachments = {};
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
    Shader* vertexShader = pipeline->GetVertexShader();

    struct PushConstantData {
        glm::mat4 mvp;
        alignas(16) glm::vec3 _color;
        alignas(16) glm::vec3 _direction;
        alignas(16) float _intensity;
        alignas(16) glm::vec3 cameraPosition;
    } data = {};

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
    std::uint32_t width = graphicsContext->GetSwapChainColorTexture()->GetWidth();
    std::uint32_t height = graphicsContext->GetSwapChainColorTexture()->GetHeight();
    
    // Camera
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::vec3 cameraPosition;
    const auto cameraView = scene->GetRegistry().view<TransformComponent, CameraComponent>();
    for(auto cameraEntity : cameraView) {
        auto [transformComponent, cameraComponent] = cameraView.get<TransformComponent, CameraComponent>(cameraEntity);
        viewMatrix = cameraComponent.m_ViewMatrix;
        projMatrix = glm::perspective(cameraComponent.m_Fov, (static_cast<float>(width) / static_cast<float>(height)),
            0.1f, 180.f);
        
        data.cameraPosition = transformComponent.m_Position;
        
        break;
    }
    
    // MVP matrix
    auto view = scene->GetRegistry().view<TransformComponent>();
    const auto& transform = view.get<TransformComponent>(entity);
    data.mvp = projMatrix * viewMatrix * transform._computedMatrix.value();

    encoder->UpdatePushConstants(pipeline, vertexShader, &data);
}

void PhongRenderPass::BindShaderResources(GraphicsContext *graphicsContext, RenderCommandEncoder *encoder, Scene *scene,
    EnttType entity) {

    // Load textures from disk and collect all shader resources
    const auto view = scene->GetRegistry().view<PhongMaterialComponent>();
    const auto& materialComponent = view.get<PhongMaterialComponent>(entity);

    if(!materialComponent._diffuseTexture || (materialComponent._diffuseTexture && !materialComponent._diffuseTexture->GetResource())) {
        assert(0);
        return;
    }

    Shader* fs = _pipeline->GetFragmentShader();
    if(!fs) {
        assert(0);
        return;
    }

    ShaderTextureResource textureResource;
    textureResource._texture = materialComponent._diffuseTexture;

    ShaderInputResource inputResource;
    inputResource._binding = fs->GetShaderResourceBinding("diffuse");
    inputResource._textureResource = textureResource;

    std::vector<ShaderInputResource> shaderResources;
    shaderResources.push_back(inputResource);

    encoder->BindShaderResources(fs, shaderResources);
}

void PhongRenderPass::Process(Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) {
    using Components = std::tuple<PrimitiveProxyComponent, PhongMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
        
    for(entt::entity entity : view) {
        BindPushConstants(encoders._renderEncoder->GetGraphicsContext(), pipeline, encoders._renderEncoder, scene, entity);
        BindShaderResources(encoders._renderEncoder->GetGraphicsContext(), encoders._renderEncoder, scene, entity);
        
        const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
        encoders._renderEncoder->DrawPrimitiveIndexed(proxy);
    }
}

ShaderInputBindings PhongRenderPass::CollectShaderInputBindings() {
    ShaderAttributeBinding vertexDataBinding = {};
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(VertexData);

    // Position vertex input
    ShaderInputLocation positions = {};
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);
    
    ShaderInputLocation normals = {};
    normals._format = Format::FORMAT_R32G32B32_SFLOAT;
    normals._offset = offsetof(VertexData, normal);

    ShaderInputLocation texCoords = {};
    texCoords._format = Format::FORMAT_R32G32_SFLOAT;
    texCoords._offset = offsetof(VertexData, texCoords);

    ShaderInputBindings inputBindings;
    inputBindings[vertexDataBinding] = {positions, normals, texCoords};
    return inputBindings;
}


std::vector<ShaderResourceBinding> PhongRenderPass::CollectResourceBindings() {
    ShaderResourceBinding matCapTexSampler2D;
    matCapTexSampler2D._id = 0;
    matCapTexSampler2D._type = ShaderInputType::TEXTURE;
    matCapTexSampler2D._shaderStage = ShaderStage::STAGE_FRAGMENT;
    matCapTexSampler2D._identifier = "diffuse";

    std::vector<ShaderResourceBinding> resourceBindings;
    resourceBindings.push_back(matCapTexSampler2D);

    return resourceBindings;
}

std::vector<PushConstant> PhongRenderPass::CollectPushConstants() {
    std::vector<PushConstant> pushConstants;
    
    PushConstant pushConstant;

    pushConstant.name = "mvp_matrix";
    pushConstant._dataType = PushConstantDataInfo<glm::mat4>::_dataType;
    pushConstant._size = PushConstantDataInfo<glm::mat4>::_gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    pushConstant.name = "lightColor";
    pushConstant._dataType = PushConstantDataInfo<glm::mat4>::_dataType;
    pushConstant._size = PushConstantDataInfo<glm::mat4>::_gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    pushConstant.name = "lightDirection";
    pushConstant._dataType = PushConstantDataInfo<glm::mat4>::_dataType;
    pushConstant._size = PushConstantDataInfo<glm::mat4>::_gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);
    
    pushConstant.name = "lightIntensity";
    pushConstant._dataType = PushConstantDataInfo<float>::_dataType;
    pushConstant._size = PushConstantDataInfo<float>::_gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);

    pushConstant.name = "cameraPosition";
    pushConstant._dataType = PushConstantDataInfo<glm::mat4>::_dataType;
    pushConstant._size = PushConstantDataInfo<glm::mat4>::_gpuSize;
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
    std::set<std::shared_ptr<Texture2D>> textures;

    for(const auto view = scene->GetRegistry().view<PhongMaterialComponent>(); const auto entity : view) {
        const auto& materialComponent = view.get<PhongMaterialComponent>(entity);
        textures.insert(materialComponent._diffuseTexture);
    }

    return textures;
}

