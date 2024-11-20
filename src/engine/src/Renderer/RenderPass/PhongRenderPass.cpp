
#include "Renderer/RenderPass/PhongRenderPass.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Shader.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/DirectionalLightComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Core/Scene.hpp"

static const char* GENERAL_DATA_BLOCK = "generalDataBlock";
static const char* PER_MODEL_DATA_BLOCK = "perModelDataBlock";
static const char* DIFFUSE_TEXTURE_BLOCK = "diffuse";

namespace ShaderStructs {
    struct GeneralData {
        float _intensity;
        alignas(16) glm::vec3 _color;
        alignas(16) glm::vec3 _direction;
        alignas(16) glm::vec3 _cameraPosition;
    };
}

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

std::vector<ShaderDataStream> PhongRenderPass::CollectShaderDataStreams() {
    std::vector<ShaderDataBlock> dataBlocks;
    
    ShaderDataBlock generalDataBlock;
    generalDataBlock._size = sizeof(ShaderStructs::GeneralData);
    generalDataBlock._usage = ShaderDataBlockUsage::UNIFORM_BUFFER;
    generalDataBlock._identifier = GENERAL_DATA_BLOCK;
    generalDataBlock._type = PushConstantDataType::PCDT_ContiguosMemory;
    generalDataBlock._stage = ShaderStage::STAGE_VERTEX;

    ShaderDataStream generalDataStream;
    generalDataStream._usage = ShaderDataStreamUsage::DATA;
    generalDataStream._dataBlocks.push_back(generalDataBlock);
    
    ShaderDataBlock perModelDataBlock;
    perModelDataBlock._size = CalculateGPUDStructSize<glm::mat4>();
    perModelDataBlock._usage = ShaderDataBlockUsage::UNIFORM_BUFFER;
    perModelDataBlock._identifier = PER_MODEL_DATA_BLOCK;
    perModelDataBlock._stage = ShaderStage::STAGE_VERTEX;

    ShaderDataStream perModelDataStream;
    perModelDataStream._usage = ShaderDataStreamUsage::DATA;
    perModelDataStream._dataBlocks.push_back(perModelDataBlock);
                        
    ShaderDataBlock diffuseDataBlock;
    diffuseDataBlock._stage = ShaderStage::STAGE_FRAGMENT;
    diffuseDataBlock._identifier = DIFFUSE_TEXTURE_BLOCK;
    diffuseDataBlock._usage = ShaderDataBlockUsage::TEXTURE;
    
    ShaderDataStream texturesDataStream;
    texturesDataStream._usage = ShaderDataStreamUsage::DATA;
    texturesDataStream._dataBlocks.push_back(diffuseDataBlock);
    
    return {generalDataStream, perModelDataStream, texturesDataStream};
}

void PhongRenderPass::Process(GraphicsContext* graphicsContext, Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) {
    using Components = std::tuple<PrimitiveProxyComponent, PhongMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
        
    // Allocate ubos
    auto dataStreams = CollectShaderDataStreams();
    for (auto& dataStream : dataStreams) {
        for(auto& block : dataStream._dataBlocks) {
            if(block._identifier == GENERAL_DATA_BLOCK) {
                if(!_generalDataBuffer) {
                    _generalDataBuffer = Buffer::Create(graphicsContext->GetDevice());
                    _generalDataBuffer->Initialize(EBufferType::BT_HOST, EBufferUsage::BU_Uniform, block._size);
                }
            }
            
            if(block._identifier == PER_MODEL_DATA_BLOCK) {
                const std::size_t requiredSize = block._size * view.handle().size() * 2;
                const std::size_t bufferSize = _perModelDataBuffer ? _perModelDataBuffer->GetSize() : 0;
                const bool bSizeChanged = (requiredSize != bufferSize);
                
                if(requiredSize > 0 && bSizeChanged) {
                    _perModelDataBuffer = Buffer::Create(graphicsContext->GetDevice());
                    _perModelDataBuffer->Initialize(EBufferType::BT_HOST, EBufferUsage::BU_Uniform, block._size * view.handle().size() * 2);
                }
            }
        }
    }
    
    unsigned int idx = 0;
    for(entt::entity entity : view) {
        BindPushConstants(encoders._renderEncoder->GetGraphicsContext(), pipeline, encoders._renderEncoder, scene, entity, idx);
        
        const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
        encoders._renderEncoder->DrawPrimitiveIndexed(proxy);
        
        idx++;
    }
}



void PhongRenderPass::BindPushConstants(GraphicsContext *graphicsContext, GraphicsPipeline *pipeline, RenderCommandEncoder *encoder, Scene *scene, EnttType entity, unsigned int entityIdx) {
    Shader* vertexShader = pipeline->GetVertexShader();
    
    // We should have a viewport abstraction that would know this type of information
    std::uint32_t width = graphicsContext->GetSwapChainColorTexture()->GetWidth();
    std::uint32_t height = graphicsContext->GetSwapChainColorTexture()->GetHeight();
        
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::mat4 mvpMatrix;
    glm::vec3 cameraPosition;
    
    // Camera data
    const auto cameraView = scene->GetRegistry().view<TransformComponent, CameraComponent>();
    for(auto cameraEntity : cameraView) {
        auto [transformComponent, cameraComponent] = cameraView.get<TransformComponent, CameraComponent>(cameraEntity);
        viewMatrix = cameraComponent.m_ViewMatrix;
        projMatrix = glm::perspective(cameraComponent.m_Fov, (static_cast<float>(width) / static_cast<float>(height)),
            0.1f, 300.f);
        
        cameraPosition = transformComponent.m_Position;
        
        break;
    }
    
    // MVP matrix
    const auto& view = scene->GetRegistry().view<TransformComponent>();
    const auto& transform = view.get<TransformComponent>(entity);
    mvpMatrix = projMatrix * viewMatrix * transform._computedMatrix.value();
    
    auto dataStreams = CollectShaderDataStreams();

    // Bind data for data stream blocks
    for (auto& dataStream : dataStreams) {
        for(auto& block : dataStream._dataBlocks) {
            if(block._identifier == GENERAL_DATA_BLOCK) {
                if(_generalDataBuffer && _generalDataBuffer->IsDirty()) {
                    ShaderStructs::GeneralData generalData;
                    generalData._cameraPosition = cameraPosition;
                    
                    // Light Data
                    const auto directionalLightView = scene->GetRegistry().view<DirectionalLightComponent>();
                    for (auto lightEntity : directionalLightView) {
                        auto& lightComponent = directionalLightView.get<DirectionalLightComponent>(lightEntity);
                            
                        generalData._color = lightComponent._color;
                        generalData._direction = lightComponent._direction;
                        generalData._intensity = lightComponent._intensity;

                        break;
                    }

                    // Copy the data to the generalBuffer
                    void* buffer = _generalDataBuffer->LockBuffer();
                    std::memset(buffer, 0, block._size); // clear memory
                    std::memcpy(buffer, &generalData, block._size);
                    _generalDataBuffer->UnlockBuffer();
                }
                
                ShaderBufferResource resource;
                resource._offset = 0;
                resource._bufferResource = _generalDataBuffer;
                
                block._data = resource;
            }
            if(block._identifier == PER_MODEL_DATA_BLOCK) {
                if(_perModelDataBuffer) {
                    // Copy the data to the generalBuffer
                    void* buffer = _perModelDataBuffer->LockBuffer();
                    std::memcpy(static_cast<char*>(buffer) + (block._size * entityIdx), &mvpMatrix, block._size);
                    _perModelDataBuffer->UnlockBuffer();
                    
                    ShaderBufferResource resource;
                    resource._offset = block._size * entityIdx;
                    resource._bufferResource = _perModelDataBuffer;
                    
                    block._data = resource;
                }
            }
            if(block._identifier == DIFFUSE_TEXTURE_BLOCK) {
                ShaderTextureResource shaderTextureResource;
                shaderTextureResource._texture = scene->GetRegistry().get<PhongMaterialComponent>(entity)._diffuseTexture;
                block._data = shaderTextureResource;
            }
        }
    }

    encoder->DispatchDataStreams(pipeline, dataStreams);
}

void PhongRenderPass::BindShaderResources(GraphicsContext *graphicsContext, RenderCommandEncoder *encoder, Scene *scene,
    EnttType entity) {

    /*
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
     */

    //encoder->BindShaderResources(fs, shaderResources);
}

/*
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
*/

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

