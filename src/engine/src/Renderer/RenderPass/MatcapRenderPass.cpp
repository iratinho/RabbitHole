#include "Renderer/RenderPass/MatcapRenderPass.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Components/MatCapMaterialComponent.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Texture2D.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/CameraComponent.hpp"

#include "Core/Scene.hpp"

static const char* GENERAL_DATA_BLOCK = "generalData";
static const char* PER_MODEL_DATA_BLOCK = "perModelData";
static const char* MATCAP_TEXTURE_BLOCK = "matcapTexture";
static const char* SAMPLER_BLOCK = "samplerBlock";

namespace ShaderStructs {
    struct alignas(256) PerModelData {
        glm::mat4 _modelViewMatrix;
        glm::mat4 _mvpMatrix;
        glm::mat4 _normalMatrix;
    };
}

RenderAttachments MatcapRenderPass::GetRenderAttachments(GraphicsContext* graphicsContext) {
    ColorAttachmentBlending blending;
    blending._colorBlending = BlendOperation::BLEND_OP_ADD;
    blending._alphaBlending = BlendOperation::BLEND_OP_ADD;
    blending._colorBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
    blending._alphaBlendingFactor = { BlendFactor::BLEND_FACTOR_ONE, BlendFactor::BLEND_FACTOR_ZERO };

    ColorAttachmentBinding colorAttachmentBinding;
    colorAttachmentBinding._texture = graphicsContext->GetGBufferTexture()._colorTexture;
    colorAttachmentBinding._blending = blending;
    colorAttachmentBinding._loadAction = LoadOp::OP_CLEAR;

    DepthStencilAttachmentBinding depthAttachmentBinding;
    depthAttachmentBinding._texture = graphicsContext->GetGBufferTexture()._depthTexture;
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
    ShaderAttributeBinding vertexDataBinding {};
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(VertexData);

    // Position vertex input
    ShaderInputLocation positions {};
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);
    
    ShaderInputLocation normals {};
    normals._format = Format::FORMAT_R32G32B32_SFLOAT;
    normals._offset = offsetof(VertexData, normal);

    ShaderInputBindings inputBindings;
    inputBindings[vertexDataBinding] = {positions, normals};
    return inputBindings;
}

std::vector<ShaderDataStream> MatcapRenderPass::CollectShaderDataStreams() {
    std::vector<ShaderDataBlock> dataBlocks;
    
    ShaderDataBlock perModelDataBlock;
    perModelDataBlock._identifier = PER_MODEL_DATA_BLOCK;
    perModelDataBlock._size = sizeof(ShaderStructs::PerModelData);
    perModelDataBlock._stage = ShaderStage::STAGE_VERTEX;
    perModelDataBlock._usage = ShaderDataBlockUsage::UNIFORM_BUFFER;
    
    ShaderDataStream dataStream;
    dataStream._usage = ShaderDataStreamUsage::DATA;
    dataStream._dataBlocks.push_back(perModelDataBlock);

    ShaderDataBlock textureDataBlock;
    textureDataBlock._identifier = MATCAP_TEXTURE_BLOCK;
    textureDataBlock._stage = ShaderStage::STAGE_FRAGMENT;
    textureDataBlock._usage = ShaderDataBlockUsage::TEXTURE;
    
    ShaderDataBlock samplerDataBlock;
    samplerDataBlock._identifier = SAMPLER_BLOCK;
    samplerDataBlock._stage = ShaderStage::STAGE_FRAGMENT;
    samplerDataBlock._usage = ShaderDataBlockUsage::SAMPLER;
    
    ShaderDataStream texturesDataStream {};
    texturesDataStream._dataBlocks.push_back(textureDataBlock);
    texturesDataStream._dataBlocks.push_back(samplerDataBlock);
    texturesDataStream._usage = ShaderDataStreamUsage::DATA;

    return { dataStream, texturesDataStream};
}

void MatcapRenderPass::BindPushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* pipeline, RenderCommandEncoder* encoder, Scene* scene, EnttType entity, unsigned int entityIdx) {
    
    Shader* fs = _pipeline->GetFragmentShader();
    if(!fs) {
        assert(0);
        return;
    }
    
    // We should have a viewport abstraction that would know this type of information
    int width = graphicsContext->GetGBufferTexture()._colorTexture->GetWidth();
    int height = graphicsContext->GetGBufferTexture()._colorTexture->GetHeight();

    // Camera
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    
    const auto cameraView = scene->GetRegistry().view<TransformComponent, CameraComponent>();
    for(auto cameraEntity : cameraView) {
        auto [transformComponent, cameraComponent] = cameraView.get<TransformComponent, CameraComponent>(cameraEntity);
        viewMatrix = cameraComponent.m_ViewMatrix;
        projMatrix = glm::perspective(cameraComponent.m_Fov, ((float)width / (float)height), 0.1f, 300.f);
        
        break;
    }
    
    auto view = scene->GetRegistry().view<TransformComponent, MatCapMaterialComponent>();
    const auto& transform = view.get<TransformComponent>(entity);
    
    glm::mat4 mvpMatrix = projMatrix * viewMatrix * transform._computedMatrix.value();
    glm::mat4 modelViewMatrix = viewMatrix * transform._computedMatrix.value();
    
    auto dataStreams = CollectShaderDataStreams();

    // Bind data for data stream blocks
    for (auto& dataStream : dataStreams) {
        for(auto& block : dataStream._dataBlocks) {
            if(block._identifier == PER_MODEL_DATA_BLOCK) {
                if(_perModelDataBuffer) {
                    ShaderStructs::PerModelData perModelData;
                    perModelData._mvpMatrix = mvpMatrix;
                    perModelData._modelViewMatrix = modelViewMatrix;
                    perModelData._normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
                    
                    // Copy the data to the generalBuffer
                    void* buffer = _perModelDataBuffer->LockBuffer();
                    std::memcpy(static_cast<char*>(buffer) + (block._size * entityIdx), &perModelData, block._size);
                    _perModelDataBuffer->UnlockBuffer();
                    
                    ShaderBufferResource resource;
                    resource._offset = block._size * entityIdx;
                    resource._bufferResource = _perModelDataBuffer;
                    
                    block._data = resource;
                }
            }
            
            if(block._identifier == MATCAP_TEXTURE_BLOCK) {
                ShaderTextureResource shaderTextureResource;
                shaderTextureResource._texture = view.get<MatCapMaterialComponent>(entity)._matCapTexture;
                block._data = shaderTextureResource;
            }
            
            if(block._identifier == SAMPLER_BLOCK) {
                ShaderTextureResource shaderTextureResource;
                shaderTextureResource._sampler = {};
                block._data = shaderTextureResource;
            }
        }
    }

    encoder->DispatchDataStreams(pipeline, dataStreams);
}

void MatcapRenderPass::BindShaderResources(GraphicsContext* graphicsContext, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) {
    // TODO Remove
}

std::string MatcapRenderPass::GetFragmentShaderPath() {
    // return COMBINE_SHADER_DIR(glsl/matcap.frag);
    //  return COMBINE_SHADER_DIR(wgsl/fragment_matcap.wgsl);
 return "assets/fragment_matcap.wgsl";
}

std::string MatcapRenderPass::GetVertexShaderPath() {
    // return COMBINE_SHADER_DIR(glsl/matcap.vert);
    //  return COMBINE_SHADER_DIR(wgsl/vertex_matcap.wgsl);
 return "assets/vertex_matcap.wgsl";
}

void MatcapRenderPass::Process(GraphicsContext* graphicsContext, Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) {
    if(!graphicsContext) {
        std::cerr << "MatcapRenderPass::Process: graphicsContext is null" << std::endl;
        return;
    }

    using Components = std::tuple<PrimitiveProxyComponent, MatCapMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
    
    auto dataStreams = CollectShaderDataStreams();
    for (auto& dataStream : dataStreams) {
        for(auto& block : dataStream._dataBlocks) {
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

std::set<std::shared_ptr<Buffer>> MatcapRenderPass::GetBufferResources(Scene* scene) {
    return {_perModelDataBuffer};
}
