#include "Renderer/RenderPass/FloorGridRenderPass.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphBuilder.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Buffer.hpp"
#include "Core/Scene.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"

static const char* VIEW_MATRIX_BLOCK = "viewmatrix";
static const char* PROJ_MATRIX_BLOCK = "projmatrix";
static const char* DATA_BLOCK = "data";

namespace {
    void PrintMatrix(const glm::mat4& matrix) {
        std::cout << "glm::mat4 values:" << std::endl;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                std::cout << matrix[col][row] << " "; // Access column-major elements
            }
            std::cout << std::endl;
        }
    }
}

FloorGridRenderPass::FloorGridRenderPass() {
}

GraphicsPipelineParams FloorGridRenderPass::GetPipelineParams() {
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_NONE;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::ALWAYS;
    
    return pipelineParams;
}

RenderAttachments FloorGridRenderPass::GetRenderAttachments(GraphicsContext *graphicsContext) {
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
    depthAttachmentBinding._stencilLoadAction = LoadOp::OP_CLEAR;
    
    RenderAttachments renderAttachments;
    renderAttachments._colorAttachmentBinding = colorAttachmentBinding;
    renderAttachments._depthStencilAttachmentBinding = depthAttachmentBinding;

    return renderAttachments;
}

void FloorGridRenderPass::Process(GraphicsContext* graphicsContext, Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) {
    using Components = std::tuple<GridMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
    
    unsigned int idx = 0;
    for(entt::entity entity : view) {
        BindPushConstants(encoders._renderEncoder->GetGraphicsContext(), pipeline, encoders._renderEncoder, scene, entity, idx);
//        BindShaderResources(encoders._renderEncoder->GetGraphicsContext(), encoders._renderEncoder, scene, entity);
//        
//        const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
//        encoders._renderEncoder->DrawPrimitiveIndexed(proxy);
                
        encoders._renderEncoder->Draw(6);
        idx++;
    }
}

std::vector<ShaderDataStream> FloorGridRenderPass::CollectShaderDataStreams() {
    std::vector<ShaderDataBlock> dataBlocks;
    
    std::size_t dataSize = sizeof(glm::mat4) * 2;
    
    ShaderDataBlock data;
    data._size = dataSize;
    data._type = PushConstantDataType::PCDT_ContiguosMemory;
    data._usage = ShaderDataBlockUsage::UNIFORM_BUFFER;
    data._identifier = DATA_BLOCK;
    data._stage = (ShaderStage)(ShaderStage::STAGE_VERTEX | ShaderStage::STAGE_FRAGMENT);
    
    ShaderDataStream dataStream;
    dataStream._dataBlocks.push_back(data);
    dataStream._usage = ShaderDataStreamUsage::DATA; // TODO Dynamic analyse if we want push constant or uniform

    return { dataStream };
}

std::vector<ShaderDataStream> FloorGridRenderPass::GetPopulatedShaderDataStreams(GraphicsContext *graphicsContext, Scene *scene) {
    using Components = std::tuple<GridMaterialComponent>;
    const auto& view = scene->GetRegistryView<Components>();
    
    struct Data {
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
    } data;
    
    const auto cameraView = scene->GetRegistry().view<CameraComponent>();
    for(auto cameraEntity : cameraView) {
        // We should have a viewport abstraction that would know this type of information
        int width = graphicsContext->GetGBufferTexture()._colorTexture->GetWidth();
        int height = graphicsContext->GetGBufferTexture()._colorTexture->GetHeight();
        
        auto cameraComponent = cameraView.get<CameraComponent>(cameraEntity);
        data.viewMatrix = cameraComponent.m_ViewMatrix;
        data.projMatrix = glm::perspective(cameraComponent.m_Fov, ((float)width / (float)height), 0.01f, 1000.f);
        break;
    }

    auto dataStreams = CollectShaderDataStreams();
    
    unsigned int idx = 0;
    for(entt::entity entity : view) {
        // Bind data for data stream blocks
        for (auto& dataStream : dataStreams) {
            for(auto& block : dataStream._dataBlocks) {
                if(block._identifier == DATA_BLOCK) {
                    if(!_dataBuffer) {
                        _dataBuffer = Buffer::Create(graphicsContext->GetDevice());
                        _dataBuffer->Initialize(EBufferType::BT_HOST, EBufferUsage::BU_Uniform, block._size);
                    };
                    
                    ShaderBufferResource resource;
                    resource._offset = 0;
                    resource._bufferResource = _dataBuffer;
                    
                    block._data = resource;
                                    
                    // Copy the data to the _dataBuffer
                    void* bufferData = _dataBuffer->LockBuffer();
                    std::memcpy(bufferData, &data, block._size);
                    _dataBuffer->UnlockBuffer();
                }
            }
        }
        
        idx++;
    }
    
    return dataStreams;
};

ShaderInputBindings FloorGridRenderPass::CollectShaderInputBindings() {
    return {};
    
    ShaderAttributeBinding vertexDataBinding;
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(glm::vec3) + sizeof(glm::vec2);
    
    // Position vertex input
    ShaderInputLocation positions;
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);

    // Position vertex input
    ShaderInputLocation coords;
    coords._format = Format::FORMAT_R32G32_SFLOAT;
    coords._offset = offsetof(VertexData, texCoords);
    
    ShaderInputBindings inputBindings;
    inputBindings[vertexDataBinding] = {positions, coords};
    return inputBindings;
}

void FloorGridRenderPass::BindPushConstants(GraphicsContext *graphicsContext, GraphicsPipeline *pipeline, RenderCommandEncoder *encoder, Scene *scene, EnttType entity, unsigned int entityIdx) {
    
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
        int width = graphicsContext->GetGBufferTexture()._colorTexture->GetWidth();
        int height = graphicsContext->GetGBufferTexture()._colorTexture->GetHeight();
        
        auto cameraComponent = cameraView.get<CameraComponent>(cameraEntity);
        data.viewMatrix = cameraComponent.m_ViewMatrix;
        data.projMatrix = glm::perspective(cameraComponent.m_Fov, ((float)width / (float)height), 0.01f, 1000.f);
        break;
    }
    
    auto dataStreams = CollectShaderDataStreams();
        
    // Bind data for data stream blocks
    for (auto& dataStream : dataStreams) {
        for(auto& block : dataStream._dataBlocks) {
            if(block._identifier == DATA_BLOCK) {
                // if(!_dataBuffer) {
                //     _dataBuffer = Buffer::Create(graphicsContext->GetDevice());
                //     _dataBuffer->Initialize(EBufferType::BT_HOST, EBufferUsage::BU_Uniform, block._size);
                // };
                
                ShaderBufferResource resource;
                resource._offset = 0;
                resource._bufferResource = _dataBuffer;
                
                block._data = resource;
                                
                // // Copy the data to the _dataBuffer
                // void* bufferData = _dataBuffer->LockBuffer();
                // std::memcpy(bufferData, &data, block._size);
                // _dataBuffer->UnlockBuffer();
            }
        }
    }
    
    encoder->DispatchDataStreams(pipeline, dataStreams);
}

std::string FloorGridRenderPass::GetFragmentShaderPath() {
     return COMBINE_SHADER_DIR(glsl/floor_grid.frag);

    //   return COMBINE_SHADER_DIR(wgsl/fragment_floor_grid.wgsl);
//return "assets/fragment_floor_grid.wgsl";
}

std::string FloorGridRenderPass::GetVertexShaderPath() {
     return COMBINE_SHADER_DIR(glsl/floor_grid.vert);
    //   return COMBINE_SHADER_DIR(wgsl/vertex_floor_grid.wgsl);
//return "assets/vertex_floor_grid.wgsl";
}

std::set<std::shared_ptr<Texture2D>> FloorGridRenderPass::GetTextureResources(Scene* scene) {
    return {};
}

std::set<std::shared_ptr<Buffer>> FloorGridRenderPass::GetBufferResources(Scene* scene) {
    return {_dataBuffer};
}

