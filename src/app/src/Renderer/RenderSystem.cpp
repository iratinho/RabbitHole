#include "Renderer/RenderSystem.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/RenderPass/OpaqueRenderPass.hpp"
#include "Renderer/RenderPass/RenderPassGenerator.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Surface.hpp"
#include "Renderer/RenderGraph/GraphBuilder.hpp"
#include "Renderer/RenderPass/FloorGridRenderPass.hpp"
#include "Renderer/RenderPass/FullScreenQuadRenderPass.hpp"
#include "Core/GeometryLoaderSystem.hpp"
#include "Core/Components/CameraComponent.hpp"
#include "Core/Components/TransformComponent.hpp"
#include "Core/Components/UserInterfaceComponent.hpp"
#include "Core/Components/DirectionalLightComponent.hpp"
#include "Core/Scene.hpp"
#include "Core/Camera.hpp"
#include "Core/MeshObject.hpp"
#include "Core/Light.hpp"
#include "window.hpp"

bool RenderSystem::Initialize(InitializationParams initialization_params)
{
    m_InitializationParams = initialization_params;
    
    _renderContext = new RenderContext(this);
    if(!_renderContext->Initialize(initialization_params)) {
        return false;
    }
    
    _frameData.resize(_renderContext->GetSwapchain()->GetSwapchainImageCount());

    _renderGraph = new RenderGraph(_renderContext);
    
    auto graphBuilder = _renderGraph->MakeGraphBuilder("GraphBuilder_For_Initialization");
    
    for (size_t i = 0; i < _frameData.size(); ++i)
    {
        // Pre-Allocate sharedPtrs
        FrameData& frameData = _frameData[i];
        frameData._commandPool = std::make_shared<CommandPool>(_renderContext);
        frameData._presentableSurface = std::make_shared<Surface>();
        frameData.sync_primitives.in_flight_fence_new = std::make_shared<Fence>(_renderContext);
        
        SurfaceCreateParams surfaceCreateParams;
        surfaceCreateParams._swapChainRenderTarget = _renderContext->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::COLOR, i);
        surfaceCreateParams._swapChainRenderTargetDepth = _renderContext->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::DEPTH, i);
        surfaceCreateParams._swapChain = _renderContext->GetSwapchain();
        surfaceCreateParams._renderContext = _renderContext;
        
        graphBuilder.AllocateSurface(frameData._presentableSurface.get(), surfaceCreateParams);
        graphBuilder.AllocateFence(frameData.sync_primitives.in_flight_fence_new.get());
        graphBuilder.AllocateCommandPool(_frameData[i]._commandPool.get());
    }

    // Ask for the initial presentable image
    graphBuilder.AcquirePresentableSurface(0);
    
    graphBuilder.Execute();
    
    _frameIndex = 0;

    VALIDATE_RETURN(CreateSyncPrimitives());
    
    return true;
}

bool RenderSystem::Process(Scene* scene) {
    _activeScene = scene;
                
    Camera currentCamera;
    if(!scene->GetActiveCamera(currentCamera)) {
        return false;
    }

    glm::vec3 cameraPosition = currentCamera.GetPosition();
    glm::mat4 viewMatrix = currentCamera.GetViewMatrix();
    float fov = currentCamera.GetFOV();

    _frameIndex = _renderContext->GetSwapchain()->GetNextPresentableImage();
    
    const glm::mat4 projectionMatrix = glm::perspective(
        fov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);
    
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    modelMatrix  = glm::rotate(modelMatrix, 90.f, glm::vec3(0.0f, 1.f, 0.0f));
    
    // Graph builders are disposable and not cached per frame, we always create a new one
    auto graphBuilder = _renderGraph->MakeGraphBuilder(fmt::format("GraphBuilder_Swapchain_{0}", _frameIndex));
    
    // Wait for the previous frame finish rendering
    graphBuilder.WaitFence(_frameData[_frameIndex].sync_primitives.in_flight_fence_new.get());
    
    // Ask to presentation engine for a swapchain image
    graphBuilder.AcquirePresentableSurface(_frameIndex);
    
    // Clear current frame fences and command buffers from the pool
    graphBuilder.ResetFence(_frameData[_frameIndex].sync_primitives.in_flight_fence_new.get());
    graphBuilder.ResetCommandPool(_frameData[_frameIndex]._commandPool.get());

    // Grab a command buffer for this frame
    graphBuilder.AllocateCommandBuffer(_frameData[_frameIndex]._commandPool.get());
    graphBuilder.EnableCommandBufferRecording(_frameData[_frameIndex]._commandPool.get());

    // Allocate buffers for geometry that haven't been initialized yet
    AllocateGeometryBuffers(&graphBuilder, _frameIndex);
    
    // Render passes
    SetupOpaqueRenderPass(&graphBuilder);
    SetupFloorGridRenderPass(&graphBuilder);
    
//#if !defined(__APPLE__)
//    FullScreenQuadPassDesc fullScreenQuadDesc;
//    fullScreenQuadDesc.sceneColor = frame_data_[frame_idx]._presentableSurface->GetRenderTarget();
//    fullScreenQuadDesc.sceneDepth = frame_data_[frame_idx]._presentableSurface->GetDepthRenderTarget();
//    fullScreenQuadDesc.texture = uiRenderTarget;
//    fullScreenQuadDesc.frameIndex = (int)frame_idx;
//    fullScreenQuadDesc._commandPool = frame_data_[frame_idx]._commandPool.get();
//    graphBuilder.MakePass<FullScreenQuadPassDesc>(&fullScreenQuadDesc);
//#endif
    
    graphBuilder.DisableCommandBufferRecording(_frameData[_frameIndex]._commandPool.get());

    SubmitCommandParams submitParams {};
    submitParams.waitSemaphore = _renderContext->GetSwapchain()->GetSyncPrimtiive(_frameIndex);
    submitParams.signalSemaphore = _frameData[_frameIndex].sync_primitives.render_finish_semaphore;
    submitParams.queueFence = _frameData[_frameIndex].sync_primitives.in_flight_fence_new.get();
    graphBuilder.SubmitCommands(_frameData[_frameIndex]._commandPool.get(), submitParams);
    
    SurfacePresentParams presentParams {};
    presentParams._frameIndex = _frameIndex;
    presentParams._waitSemaphore = _frameData[_frameIndex].sync_primitives.render_finish_semaphore;
    graphBuilder.Present(_frameData[_frameIndex]._presentableSurface.get(), presentParams);
    
    bool bWasSuccessfully = graphBuilder.Execute();

    if(!bWasSuccessfully) {
        return false;
    }
    
    return true;
}

void RenderSystem::HandleResize(int width, int height) {
    _renderContext->MarkSwapchainDirty();
}

bool RenderSystem::CreateSyncPrimitives() {
    if (_renderContext) {
        VkResult result;

        for (size_t i = 0; i < _frameData.size(); ++i) {
            SyncPrimitives sync_primitives;

            // Semaphore that will be signaled when when the first command buffer finish execution
            {
                VkSemaphoreCreateInfo command_buffer_finished_semaphore_create_info;
                command_buffer_finished_semaphore_create_info.flags = 0;
                command_buffer_finished_semaphore_create_info.pNext = nullptr;
                command_buffer_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                result = VkFunc::vkCreateSemaphore(_renderContext->GetLogicalDeviceHandle(),
                                           &command_buffer_finished_semaphore_create_info, nullptr,
                                           &sync_primitives.render_finish_semaphore);

                if (result != VK_SUCCESS) {
                    return false;
                }
            }
            
            _frameData[i].sync_primitives.render_finish_semaphore = sync_primitives.render_finish_semaphore;
        }

        return true;
    }

    return false;
}

bool RenderSystem::ReleaseResources() {
    _renderGraph->ReleaseRenderTargets();
    _renderGraph->ReleasePassResources();
    return true;
}

void RenderSystem::AllocateGeometryBuffers(GraphBuilder* graphBuilder, unsigned frameIndex) {
    MeshComponent* sceneComponent = nullptr;

    for (auto view = _activeScene->GetRegistry().view<MeshComponent>(); auto entity : view) {
        sceneComponent = const_cast<MeshComponent*>(&view.get<MeshComponent>(entity));
        break;
    }
    
    if(sceneComponent && !sceneComponent->_meshNodes.empty()) {
        for (MeshNode& meshNode : sceneComponent->_meshNodes) {
            GeometryLoaderSystem::ForEachNode(&meshNode, [this, &graphBuilder, frameIndex](MeshNode* currentNode) {
                if(!currentNode->_bWasProcessed) {
                    if(currentNode->_primitives.empty()) {
                        currentNode->_bWasProcessed = true;
                        return;
                    }
                    currentNode->_buffer = std::make_shared<Buffer>(_renderContext);
                    graphBuilder->CopyGeometryData(currentNode->_buffer.get(), currentNode);
                    graphBuilder->UploadBufferData(currentNode->_buffer.get(), _frameData[frameIndex]._commandPool.get());
                    currentNode->_bWasProcessed = true;
                }
            });
        }
    }
}

void RenderSystem::SetupOpaqueRenderPass(GraphBuilder* graphBuilder) {
    Camera currentCamera;
    if(!_activeScene->GetActiveCamera(currentCamera)) {
        return false;
    }

    glm::vec3 cameraPosition = currentCamera.GetPosition();
    glm::mat4 viewMatrix = currentCamera.GetViewMatrix();
    float cameraFov = currentCamera.GetFOV();

    RenderPassGenerator opaquePassGenerator;

    ShaderConfiguration &vertexShader = opaquePassGenerator.ConfigureShader(ShaderStage::STAGE_VERTEX);
    vertexShader._shaderPath = COMBINE_SHADER_DIR(dummy.vert);

    ShaderConfiguration &fragmentShader = opaquePassGenerator.ConfigureShader(ShaderStage::STAGE_FRAGMENT);
    fragmentShader._shaderPath = COMBINE_SHADER_DIR(dummy.frag);

    RasterizationConfiguration &rasterizationConfig = opaquePassGenerator.ConfigureRasterizationOptions();
    rasterizationConfig._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    rasterizationConfig._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    rasterizationConfig._depthCompareOP = CompareOperation::LESS_OR_EQUAL;

    AttachmentConfiguration &colorAttachment = opaquePassGenerator.MakeAttachment();
    colorAttachment._attachment._loadOp = AttachmentLoadOp::OP_CLEAR;
    colorAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    colorAttachment._attachment._stencilLoadOp = AttachmentLoadOp::OP_DONT_CARE;
    colorAttachment._attachment._stencilStoreOp = AttachmentStoreOp::OP_DONT_CARE;
    colorAttachment._renderTarget = _frameData[_frameIndex]._presentableSurface->GetRenderTarget();
    colorAttachment._attachment._format = Format::FORMAT_B8G8R8A8_SRGB;
    colorAttachment._initialLayout = ImageLayout::LAYOUT_UNDEFINED;
    colorAttachment._finalLayout = ImageLayout::LAYOUT_COLOR_ATTACHMENT;

    AttachmentConfiguration &depthAttachment = opaquePassGenerator.MakeAttachment();
    depthAttachment._attachment._loadOp = AttachmentLoadOp::OP_CLEAR;
    depthAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    depthAttachment._renderTarget = _frameData[_frameIndex]._presentableSurface->GetDepthRenderTarget();
    depthAttachment._attachment._format = Format::FORMAT_D32_SFLOAT;
    depthAttachment._initialLayout = ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;
    depthAttachment._finalLayout = ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;

    ShaderInputGroup& vertexBufferInput = opaquePassGenerator.MakeShaderInputGroup();
    vertexBufferInput._stride = sizeof(VertexData);
    
    ShaderInput& vertexPositionInput = opaquePassGenerator.MakeShaderInput(vertexBufferInput, offsetof(VertexData, position));
    vertexPositionInput._format = Format::FORMAT_R32G32B32_SFLOAT;
    
    ShaderInput& vertexNormalInput = opaquePassGenerator.MakeShaderInput(vertexBufferInput, offsetof(VertexData, normal));
    vertexNormalInput._format = Format::FORMAT_R32G32B32_SFLOAT;
        
    PushConstantConfiguration* mvpPushConstant = opaquePassGenerator.MakePushConstant();
    mvpPushConstant->_pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    mvpPushConstant->size = CalculateGPUDStructSize<glm::mat4>();
    mvpPushConstant->_debugType = "mat4";
    
    Light* light = _activeScene->GetLight();
    auto lightComponent = light->GetComponents();
    PushConstantConfiguration* lightPushConstant = opaquePassGenerator.MakePushConstant();
    lightPushConstant->_pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    lightPushConstant->size = CalculateGPUDStructSize<DirectionalLightComponent>();
    lightPushConstant->_data.push_back(MakePaddedGPUBuffer(lightComponent));
    lightPushConstant->_debugType = "DirectionalLightComponent";
    
    PushConstantConfiguration* cameraPositionPushConstant = opaquePassGenerator.MakePushConstant();
    cameraPositionPushConstant->_pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    cameraPositionPushConstant->size = CalculateGPUDStructSize<glm::vec3>();
    cameraPositionPushConstant->_data.push_back(MakePaddedGPUBuffer(cameraPosition));
    cameraPositionPushConstant->_debugType = "vec3";

    // Create primitive input descriptors
    GenerateSceneProxies(&opaquePassGenerator, [&](const MeshNode* meshNode, const PrimitiveData* primitiveData) {
        const glm::mat4 projectionMatrix = glm::perspective(
            cameraFov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);
        
        const glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * meshNode->_computedMatrix;

        mvpPushConstant->_data.push_back(MakePaddedGPUBuffer(mvpMatrix));
    },
    // Filter
    [](const Mesh* mesh) {
        auto [meshComponent, transformComponent] = mesh->GetComponents();
        return meshComponent._renderMainPass;
    });

    graphBuilder->AddPass(_renderContext, _frameData[_frameIndex]._commandPool.get(), opaquePassGenerator, _frameIndex, "OpaquePass");
}

// TODO WIP - not finalized
void RenderSystem::SetupFloorGridRenderPass(GraphBuilder* graphBuilder) {
    Camera currentCamera;
    if(!_activeScene->GetActiveCamera(currentCamera)) {
        return false;
    }

    float cameraFov = currentCamera.GetFOV();
    glm::mat4 viewMatrix = currentCamera.GetViewMatrix();

    const glm::mat4 projectionMatrix = glm::perspective(
        cameraFov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);
    
    RenderPassGenerator renderPassGenerator;
    
    ShaderConfiguration &vertexShader = renderPassGenerator.ConfigureShader(ShaderStage::STAGE_VERTEX);
    vertexShader._shaderPath = COMBINE_SHADER_DIR(floor_grid.vert);

    ShaderConfiguration &fragmentShader = renderPassGenerator.ConfigureShader(ShaderStage::STAGE_FRAGMENT);
    fragmentShader._shaderPath = COMBINE_SHADER_DIR(floor_grid.frag);

    RasterizationConfiguration &rasterizationConfig = renderPassGenerator.ConfigureRasterizationOptions();
    rasterizationConfig._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    rasterizationConfig._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    rasterizationConfig._depthCompareOP = CompareOperation::LESS;
    
    AttachmentConfiguration &colorAttachment = renderPassGenerator.MakeAttachment();
    colorAttachment._attachment._loadOp = AttachmentLoadOp::OP_LOAD;
    colorAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    colorAttachment._attachment._stencilLoadOp = AttachmentLoadOp::OP_DONT_CARE;
    colorAttachment._attachment._stencilStoreOp = AttachmentStoreOp::OP_DONT_CARE;
    colorAttachment._attachment.bBlendEnabled = true;
    colorAttachment._attachment._srcColorBlendFactor = BlendFactor::BLEND_FACTOR_SRC_ALPHA;
    colorAttachment._attachment._dstColorBlendFactor = BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorAttachment._attachment._colorBlendOp = BlendOperation::BLEND_OP_ADD;
    colorAttachment._attachment._srcAlphaBlendFactor = BlendFactor::BLEND_FACTOR_ONE;
    colorAttachment._attachment._dstAlphaBlendFactor = BlendFactor::BLEND_FACTOR_ZERO;
    colorAttachment._attachment._alphaBlendOp = BlendOperation::BLEND_OP_ADD;
    colorAttachment._renderTarget = _frameData[_frameIndex]._presentableSurface->GetRenderTarget();
    colorAttachment._attachment._format = Format::FORMAT_B8G8R8A8_SRGB;
    colorAttachment._initialLayout = ImageLayout::LAYOUT_COLOR_ATTACHMENT;
    colorAttachment._finalLayout = ImageLayout::LAYOUT_PRESENT;

    AttachmentConfiguration &depthAttachment = renderPassGenerator.MakeAttachment();
    depthAttachment._attachment._loadOp = AttachmentLoadOp::OP_LOAD;
    depthAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    depthAttachment._renderTarget = _frameData[_frameIndex]._presentableSurface->GetDepthRenderTarget();
    depthAttachment._attachment._format = Format::FORMAT_D32_SFLOAT;
    depthAttachment._initialLayout = ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;
    depthAttachment._finalLayout = ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;
        
    // view matrix
    PushConstantConfiguration* viewMatrixPushConstant = renderPassGenerator.MakePushConstant();
    viewMatrixPushConstant->_pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    viewMatrixPushConstant->size = CalculateGPUDStructSize<glm::mat4>();
    viewMatrixPushConstant->_data.push_back(MakePaddedGPUBuffer(viewMatrix));
    viewMatrixPushConstant->_debugType = "mat4";
    
    // projection matrix
    PushConstantConfiguration* projPushConstant = renderPassGenerator.MakePushConstant();
    projPushConstant->_pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    projPushConstant->size = CalculateGPUDStructSize<glm::mat4>();
    projPushConstant->_data.push_back(MakePaddedGPUBuffer(projectionMatrix));
    projPushConstant->_debugType = "mat4";
        
    ShaderInputGroup& vertexBufferInput = renderPassGenerator.MakeShaderInputGroup();
    vertexBufferInput._stride = sizeof(VertexData);
    
    ShaderInput& vertexPositionInput = renderPassGenerator.MakeShaderInput(vertexBufferInput, offsetof(VertexData, position));
    vertexPositionInput._format = Format::FORMAT_R32G32B32_SFLOAT;

    ShaderInput& vertexNormalInput = renderPassGenerator.MakeShaderInput(vertexBufferInput, offsetof(VertexData, normal));
    vertexNormalInput._format = Format::FORMAT_R32G32B32_SFLOAT;

    // Create primitive input descriptors
    GenerateSceneProxies(&renderPassGenerator, [&](const MeshNode*, const PrimitiveData* primitiveData) {},
    [](const Mesh* mesh) {
        auto [meshComponent, transformComponent] = mesh->GetComponents();
        return meshComponent._renderFloorGridPass;
    });

    graphBuilder->AddPass(_renderContext, _frameData[_frameIndex]._commandPool.get(), renderPassGenerator,_frameIndex, "FloorGridPass");
}

void RenderSystem::GenerateSceneProxies(RenderPassGenerator* renderPassGenerator, std::function<void(const MeshNode*, const PrimitiveData*)> func, std::function<bool(const Mesh*)> filter) {
    if(_activeScene) {
        _activeScene->ForEachMesh([&, this](Mesh* mesh) {
            if(mesh) {
                if(filter(mesh)) {
                    mesh->ComputeMatrix();
                    mesh->ForEachNode([&, this](const MeshNode * meshNode) {
                        for(const PrimitiveData& primitiveData : meshNode->_primitives) {
                            // This is not yet correct, i also need a way to filter primitives for passes
                            PrimitiveProxy& primitiveDataDescription = renderPassGenerator->MakePrimitiveProxy();
                            primitiveDataDescription._indicesCount = primitiveData._indices.size();
                            primitiveDataDescription._indicesBufferOffset = primitiveData._indicesOffset;
                            primitiveDataDescription._vOffset.push_back(primitiveData._vertexOffset);
                            primitiveDataDescription._primitiveBuffer = meshNode->_buffer;
                        
                            func(meshNode, &primitiveData);
                        }
                    });
                }
            }
        });
    }
}

