#include "Components/DirectionalLightComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
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
    SetupFloorGridRenderPass(graphicsContext, scene);
    graphicsContext->Flush();
}

// How to improve locality in this situation
void RenderSystemV2::ProcessGeometry(Scene *scene) {
    std::vector<std::shared_ptr<Buffer>> buffers;
    
    auto hasGPUData = [scene](entt::entity entity) -> bool {
        return scene->GetRegistry().any_of<PrimitiveProxyComponent>(entity);
    };
    
    auto calculateAllocationSize = [scene](entt::entity entity) -> size_t {
        PrimitiveProxyComponentCPU& cpuComponent = scene->GetRegistry().get<PrimitiveProxyComponentCPU>(entity);
        return cpuComponent._indices.size() * sizeof(unsigned) + cpuComponent._vertexData.size() * sizeof(VertexData);
    };
    
    auto copyPrimitiveData = [scene](entt::entity entity, void* bufferAlloc, size_t& allocationSize) -> std::tuple<unsigned int, unsigned int> {
        PrimitiveProxyComponentCPU& cpuComponent = scene->GetRegistry().get<PrimitiveProxyComponentCPU>(entity);
        
        memcpy(bufferAlloc, cpuComponent._indices.data(), cpuComponent._indices.size() * sizeof(unsigned int));
        memcpy((unsigned int*)bufferAlloc + cpuComponent._indices.size(),
            cpuComponent._vertexData.data(),
            cpuComponent._vertexData.size() * sizeof(VertexData));
        
        return { cpuComponent._indices.size(), cpuComponent._vertexData.size()};
    };
    
    std::function<void(entt::entity)> forEachMesh;
    forEachMesh = [&, forEachMesh](entt::entity entity) {
        MeshComponentNew& meshComponent = scene->GetRegistry().get<MeshComponentNew>(entity);

        // Calculate how much data we need to allocate for all primitives
        size_t allocationSize = 0;
        for(auto childEntity : meshComponent._primitives) {
                allocationSize += !hasGPUData(childEntity) ? calculateAllocationSize(childEntity) : 0;
        }
        
        if(allocationSize == 0) {
            return;
        }
        
        auto buffer = Buffer::Create(_device.get(), (EBufferType)(EBufferType::BT_HOST | EBufferType::BT_LOCAL), (EBufferUsage)(EBufferUsage::BU_Geometry | EBufferUsage::BU_Transfer), allocationSize);
        buffer->Initialize();
        
        void* bufferAlloc = (unsigned char*)buffer->LockBuffer();
        
        
        int pCount = 0;
        for(auto childEntity : meshComponent._primitives) {
            PrimitiveProxyComponentCPU& cpuComponent = scene->GetRegistry().get<PrimitiveProxyComponentCPU>(childEntity);
            auto [iSize, vSize] = copyPrimitiveData(childEntity, bufferAlloc, allocationSize);
            
            
            scene->GetRegistry().emplace<PrimitiveProxyComponent>(childEntity);
            
            PrimitiveProxyComponent& proxy = scene->GetRegistry().get<PrimitiveProxyComponent>(childEntity);
            proxy._gpuBuffer = buffer;
            proxy._indicesCount = cpuComponent._indices.size();
            proxy._indicesOffset = pCount * iSize;
            proxy._vertexOffset = pCount * vSize;
            
            pCount = std::min(++pCount, 1);
        }
        
        buffer->UnlockBuffer();
        
        buffers.push_back(buffer);

        for(auto childEntity : meshComponent._meshes) {
            forEachMesh(childEntity);
        }
    };

    for(auto entity : scene->GetRegistry().view<MeshComponentNew>()) {
        forEachMesh(entity);
    }

    for (auto buffer : buffers) {
        _transferContext->EnqueueBufferSync(buffer);
    }
    
    if(buffers.size() > 0) {
        _transferContext->Flush();
    }
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

bool RenderSystemV2::SetupFloorGridRenderPass(GraphicsContext* graphicsContext, Scene* scene) {
    // Binding 0 for vertex data
    ShaderInputBinding vertexDataBinding;
    vertexDataBinding._binding = 0;
    vertexDataBinding._stride = sizeof(VertexData);
    
    // Position vertex input
    ShaderInputLocation positions;
    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
    positions._offset = offsetof(VertexData, position);
        
    auto vertexShader = Shader::MakeShader(_device.get(), COMBINE_SHADER_DIR(floor_grid_vert.spv), ShaderStage::STAGE_VERTEX);
    vertexShader->DeclareShaderBindingLayout(vertexDataBinding, { positions });
    vertexShader->DeclarePushConstant<glm::mat4>("viewMatrix");
    vertexShader->DeclarePushConstant<glm::mat4>("projMatrix");

    auto fragmentShader = Shader::MakeShader(_device.get(), COMBINE_SHADER_DIR(floor_grid_frag.spv), ShaderStage::STAGE_FRAGMENT);
    fragmentShader->DeclareShaderOutput("");
        
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::LESS;
    pipelineParams._vertexShader = vertexShader;
    pipelineParams._fragmentShader = fragmentShader;
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
    
    auto render = [scene, graphicsContext, vertexShader, fragmentShader](class CommandEncoder* encoder, class GraphicsPipeline* pipeline) {
        // We should have a viewport abstraction that would know this type of information
        int width = graphicsContext->GetSwapchainColorTarget()->GetWidth();
        int height = graphicsContext->GetSwapchainColorTarget()->GetHeight();
        
        // Extract the viewMatrix from the camera
        const auto cameraView = scene->GetRegistry().view<CameraComponent>();
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
        if(cameraView.size() > 0) {
            const auto& cameraComponent = cameraView.get<CameraComponent>(cameraView[0]);
            viewMatrix = cameraComponent.m_ViewMatrix;
            projMatrix = glm::perspective(
                cameraComponent.m_Fov, ((float)width / (float)height), 0.1f, 180.f);
        }
        
        const auto& view = scene->GetRegistry().view<PrimitiveProxyComponent, GridMaterialComponent>();
        for(auto entity : view) {
            const auto& [proxy, material] = view.get<PrimitiveProxyComponent, GridMaterialComponent>(entity);
            encoder->UpdatePushConstant(graphicsContext, pipeline, vertexShader.get(), "viewMatrix", &viewMatrix);
            encoder->UpdatePushConstant(graphicsContext, pipeline, vertexShader.get(), "projMatrix", &projMatrix);
            encoder->DrawPrimitiveIndexed(graphicsContext, proxy);
        }
    };
    
    graphicsContext->GetGraphBuilder().AddRasterPass(pipelineParams, renderAttachments, render);

    return true;
}
