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
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_BYTE_CODE_DIR) "/" STR(name)

// TODO
// Fix StaticOPs
// Fix pipeline creation based on parameters
// Shader Storage (with caching)
// PipelineStorage (with caching)
// Reorganize VKGraphicsContext to VKCommandListContext class

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
    // Upload to GPU side all geometry resources
    ProcessGeometry(scene);
        
    // Till we have a proper ring buffer
    if(currentContext > 1) {
        currentContext = 0;
    }
    
    auto& graphicsContext = _graphicsContext[currentContext];
    if(!graphicsContext) {
        return false;
    }
    
    graphicsContext->BeginFrame();
    Render(graphicsContext.get(), scene);
    graphicsContext->EndFrame();
    graphicsContext->Present();
    
    currentContext++;
    
    return true;
}

void RenderSystemV2::Render(GraphicsContext* graphicsContext, Scene* scene) {
    std::vector<PrimitiveProxy> proxies;
    
    // 1º Iterate over all meshes
    scene->ForEachMesh([&](Mesh* mesh) {
        // 2ª Compute mesh child nodes matrices
        mesh->ComputeMatrix();
        // 3º Iterate over all mesh nodes to build render thread information about a mesh
        mesh->ForEachNode([&](const MeshNode* meshNode) {
            if(!meshNode->_buffer) {
               return;
            }
            // 4ª Each mesh node contains mesh primtives lets get their data
            for(const PrimitiveData& primitiveData : meshNode->_primitives) {
                // 5º Build the mesh proxy data
                PrimitiveProxy proxy;
                proxy._indicesCount = static_cast<unsigned int>(primitiveData._indices.size());
                proxy._indicesBufferOffset = primitiveData._indicesOffset;
                proxy._vOffset.push_back(primitiveData._vertexOffset);
                proxy._primitiveBuffer = meshNode->_buffer;
                proxy._transformMatrix = meshNode->_computedMatrix; // TODO This should be at primitive level
                proxy._viewportX = graphicsContext->GetSwapchainColorTarget()->GetWidth(); // ???
                proxy._viewportY = graphicsContext->GetSwapchainColorTarget()->GetHeight(); // ??
                proxy._primitiveScene = scene;
                proxies.push_back(proxy);
            }
        });
    });

    SetupFloorGridRenderPass(graphicsContext);
    
    graphicsContext->Flush();
}

void RenderSystemV2::ProcessGeometry(Scene *scene) {
    scene->ForEachMesh([this](Mesh* mesh) {
        mesh->ForEachNode([this](MeshNode* meshNode) {
            // If buffer is null it means we never created GPU data for this mesh primitive
            if(!meshNode->_buffer) {
                // Compute the necessary allocation size for our buffers
                size_t allocationSize = 0;
                for (const PrimitiveData& primitiveData : meshNode->_primitives) {
                    allocationSize += primitiveData._indices.size() * sizeof(unsigned) + primitiveData._vertexData.size() * sizeof(VertexData);
                }

                meshNode->_buffer = Buffer::Create(_device.get(), (EBufferType)(EBufferType::BT_HOST | EBufferType::BT_LOCAL), (EBufferUsage)(EBufferUsage::BU_Geometry | EBufferUsage::BU_Transfer), allocationSize);
                
                meshNode->_buffer->Initialize();
            
                unsigned char* bufferAlloc = (unsigned char*)meshNode->_buffer->LockBuffer();
                
                if(bufferAlloc) {
                    // Copy data to staging buffer
                    for (const PrimitiveData& primitiveData : meshNode->_primitives) {
                        // Copy Indices
                        memcpy(bufferAlloc, primitiveData._indices.data(), primitiveData._indices.size() * sizeof(unsigned));
                        
                        // Copy VertexData
                        memcpy(bufferAlloc + primitiveData._indices.size() * sizeof(unsigned),
                            primitiveData._vertexData.data(),
                            primitiveData._vertexData.size() * sizeof(VertexData));
                    }
                }
                
                meshNode->_buffer->UnlockBuffer();
                
                _transferContext->EnqueueBufferSync(meshNode->_buffer);
            }
        });
    });
    
    // Flush all geometry buffers transfer operations
    _transferContext->Flush();
}

// TODO create static classes in a render pass file to hold this data and the draw function too
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

bool RenderSystemV2::SetupFloorGridRenderPass(GraphicsContext* graphicsContext) {
    // Binding 0 for vertex data
    ShaderInputBinding vertexDataBinding;
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(VertexData);
    
    // Position vertex input
    ShaderInputLocation positions;
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);
        
    auto vertexShader = Shader::MakeShader(_device.get(), COMBINE_SHADER_DIR(floor_grid.vert), ShaderStage::STAGE_VERTEX);
    vertexShader->DeclareShaderBindingLayout(vertexDataBinding, { positions });
    vertexShader->DeclarePushConstant<glm::mat4>("viewMatrix");
    vertexShader->DeclarePushConstant<glm::mat4>("projMatrix");

    auto fragmentShader = Shader::MakeShader(_device.get(), COMBINE_SHADER_DIR(floor_grid.frag), ShaderStage::STAGE_FRAGMENT);
    fragmentShader->DeclareShaderOutput("");
        
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;
    pipelineParams._vertexShader = vertexShader;
    pipelineParams._fragmentShader = fragmentShader;
    pipelineParams._id = currentContext;
    
    RasterPassTarget colorTarget;
    colorTarget._attachmentInfo._format = Format::FORMAT_B8G8R8A8_SRGB;
    colorTarget._attachmentInfo._blendOp  = Ops::StaticBlendOp<BlendOperation::BLEND_OP_ADD, BlendOperation::BLEND_OP_ADD>::Get();
    colorTarget._attachmentInfo._loadStoreOp = Ops::StaticOp<AttachmentLoadOp::OP_DONT_CARE, AttachmentStoreOp::OP_STORE, AttachmentLoadOp::OP_DONT_CARE, AttachmentStoreOp::OP_DONT_CARE>::Get();
    colorTarget._attachmentInfo._blendFactorOp = Ops::StaticBlendFactorOp<BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE, BlendFactor::BLEND_FACTOR_ZERO>::Get();
    colorTarget._renderTarget = graphicsContext->GetSwapchainColorTarget();
    colorTarget._attachmentInfo._layoutOp = Ops::StaticLayoutOp<ImageLayout::LAYOUT_COLOR_ATTACHMENT, ImageLayout::LAYOUT_COLOR_ATTACHMENT>::Get(); // Should this really be an OP? We can rename it to _usage and only accept the dest since we can track the current in the textures. Also
    // how to deal with AccessFlags? it seems metal API and DX dont have this... Also should i care about this stuff? Can this be infered
    // with the pass dependencies? Might be more complicated tho, but it would be nice
    colorTarget._clearColor = glm::vec3(1.0, 1.0, 1.0);
    
    // I think its time to work on creating a node graph class for all of this
    
    RasterPassTarget depthTarget;
    depthTarget._attachmentInfo._loadStoreOp = Ops::StaticOp<AttachmentLoadOp::OP_LOAD, AttachmentStoreOp::OP_STORE, AttachmentLoadOp::OP_DONT_CARE, AttachmentStoreOp::OP_DONT_CARE>::Get();
    depthTarget._attachmentInfo._format = Format::FORMAT_D32_SFLOAT;
    depthTarget._renderTarget = graphicsContext->GetSwapchainDepthTarget();
    depthTarget._attachmentInfo._layoutOp = Ops::StaticLayoutOp<ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT, ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT>::Get();
    depthTarget._clearColor = glm::vec3(1.0, 1.0, 1.0);

    graphicsContext->GetGraphBuilder().AddRasterPass(pipelineParams, colorTarget, depthTarget, [](CommandEncoder* encoder) {
        encoder->SetViewport();
    });

    return true;
}
