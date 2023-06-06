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
        
        graphBuilder.AllocateSurface(frameData._presentableSurface, surfaceCreateParams);
        graphBuilder.AllocateFence(frameData.sync_primitives.in_flight_fence_new);
        graphBuilder.AllocateCommandPool(_frameData[i]._commandPool.get());
    }

    // Ask for the initial presentable image
    graphBuilder.AcquirePresentableSurface(0);
    
    graphBuilder.Execute();
    
    _frameIndex = 0;

    VALIDATE_RETURN(CreateSyncPrimitives());
    
    return true;
}

bool RenderSystem::Process(const entt::registry& registry) {
    // Find the main view entity to extract camera matrix and calculate a projection matrix
    float fov;
    glm::vec3 cameraPosition;
    glm::mat4 viewMatrix;
    std::shared_ptr<RenderTarget> uiRenderTarget;
    const MeshComponent* sceneComponentPtr = nullptr;

    auto view = registry.view<const TransformComponent, const CameraComponent, const UserInterfaceComponent, const MeshComponent>();
    
    for (auto entity : view) {
        auto [transformComponent, cameraComponent, userInterfaceComponent, sceneComponent] = view.get<TransformComponent, CameraComponent, UserInterfaceComponent, MeshComponent>(entity);
        
        cameraPosition = transformComponent.m_Position;
        viewMatrix = cameraComponent.m_ViewMatrix;
        fov = cameraComponent.m_Fov;
        uiRenderTarget = userInterfaceComponent._uiRenderTarget;

        sceneComponentPtr = &sceneComponent;
        
        break;
    }

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
    AllocateGeometryBuffers(registry, &graphBuilder, _frameIndex);
    
    // Render passes
    SetupOpaqueRenderPass(&graphBuilder, registry);
//    SetupFloorGridRenderPass(&graphBuilder, registry);
    
    /*
    FloorGridPassDesc floorGridPassDesc {};
    floorGridPassDesc.scene_color = _frameData[_frameIndex]._presentableSurface->GetRenderTarget();
    floorGridPassDesc.scene_depth = _frameData[_frameIndex]._presentableSurface->GetDepthRenderTarget();
    floorGridPassDesc.previousPassIndex = VK_SUBPASS_EXTERNAL;
    floorGridPassDesc.nextPassIndex = 0;
    floorGridPassDesc.enabled_ = true;
    floorGridPassDesc.frameIndex = (int)_frameIndex;
    floorGridPassDesc.viewMatrix = viewMatrix;
    floorGridPassDesc.projectionMatrix = projectionMatrix;
    floorGridPassDesc._commandPool = _frameData[_frameIndex]._commandPool.get();
    graphBuilder.MakePass<FloorGridPassDesc>(&floorGridPassDesc);
     */

#if !defined(__APPLE__)
    FullScreenQuadPassDesc fullScreenQuadDesc;
    fullScreenQuadDesc.sceneColor = frame_data_[frame_idx]._presentableSurface->GetRenderTarget();
    fullScreenQuadDesc.sceneDepth = frame_data_[frame_idx]._presentableSurface->GetDepthRenderTarget();
    fullScreenQuadDesc.texture = uiRenderTarget;
    fullScreenQuadDesc.frameIndex = (int)frame_idx;
    fullScreenQuadDesc._commandPool = frame_data_[frame_idx]._commandPool.get();
    graphBuilder.MakePass<FullScreenQuadPassDesc>(&fullScreenQuadDesc);
#endif
    
    graphBuilder.DisableCommandBufferRecording(_frameData[_frameIndex]._commandPool.get());

    SubmitCommandParams submitParams {};
    submitParams.waitSemaphore = _renderContext->GetSwapchain()->GetSyncPrimtiive(_frameIndex);
    submitParams.signalSemaphore = _frameData[_frameIndex].sync_primitives.render_finish_semaphore;
    submitParams.queueFence = _frameData[_frameIndex].sync_primitives.in_flight_fence_new.get();
    graphBuilder.SubmitCommands(_frameData[_frameIndex]._commandPool.get(), submitParams);
    
    SurfacePresentParams presentParams {};
    presentParams._frameIndex = _frameIndex;
    presentParams._waitSemaphore = _frameData[_frameIndex].sync_primitives.render_finish_semaphore;
    graphBuilder.Present(_frameData[_frameIndex]._presentableSurface, presentParams);
    
    bool bWasSuccessfully = graphBuilder.Execute();

    if(!bWasSuccessfully) {
        return false;
    }
    
    return true;
}

void RenderSystem::HandleResize(int width, int height)
{
    _renderContext->MarkSwapchainDirty();
}

bool RenderSystem::CreateSyncPrimitives()
{
    if (_renderContext)
    {
        VkResult result;

        for (size_t i = 0; i < _frameData.size(); ++i)
        {
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

                if (result != VK_SUCCESS)
                {
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

void RenderSystem::AllocateGeometryBuffers(const entt::registry& registry, GraphBuilder* graphBuilder, unsigned frameIndex)
{
    MeshComponent* sceneComponent = nullptr;

    for (auto view = registry.view<MeshComponent>(); auto entity : view) {
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
                    graphBuilder->CopyGeometryData(currentNode->_buffer, currentNode);
                    graphBuilder->UploadBufferData(currentNode->_buffer, _frameData[frameIndex]._commandPool->GetCommandBuffer().get());
                    currentNode->_bWasProcessed = true;
                }
            });
        }
    }
}

void RenderSystem::SetupOpaqueRenderPass(GraphBuilder* graphBuilder, const entt::registry& registry) {
    auto view = registry.view<const CameraComponent, const MeshComponent>();
    
    float cameraFov;
    glm::mat4 viewMatrix;
    const MeshComponent* meshComponentPtr = nullptr;
    for (auto entity : view) {
        auto [cameraComponent, meshComponent] = view.get<CameraComponent, MeshComponent>(entity);
        viewMatrix = cameraComponent.m_ViewMatrix;
        cameraFov = cameraComponent.m_Fov;
        meshComponentPtr = &meshComponent;
        break;
    }
        
    RenderPassGenerator opaquePassGenerator;

    ShaderConfiguration &vertexShader = opaquePassGenerator.ConfigureShader(ShaderStage::STAGE_VERTEX);
    vertexShader._shaderPath = COMBINE_SHADER_DIR(dummy_vs.spv);

    ShaderConfiguration &fragmentShader = opaquePassGenerator.ConfigureShader(ShaderStage::STAGE_FRAGMENT);
    fragmentShader._shaderPath = COMBINE_SHADER_DIR(dummy_fs.spv);

    RasterizationConfiguration &rasterizationConfig = opaquePassGenerator.ConfigureRasterizationOptions();
    rasterizationConfig._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    rasterizationConfig._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;

    AttachmentConfiguration &colorAttachment = opaquePassGenerator.MakeAttachment();
    colorAttachment._attachment._loadOp = AttachmentLoadOp::OP_CLEAR;
    colorAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    colorAttachment._attachment._stencilLoadOp = AttachmentLoadOp::OP_DONT_CARE;
    colorAttachment._attachment._stencilStoreOp = AttachmentStoreOp::OP_DONT_CARE;
    colorAttachment._renderTarget = _frameData[_frameIndex]._presentableSurface->GetRenderTarget();
    colorAttachment._attachment._format = Format::FORMAT_B8G8R8A8_SRGB;
    colorAttachment._initialLayout = ImageLayout::LAYOUT_COLOR_ATTACHMENT;
//    colorAttachment._finalLayout = ImageLayout::LAYOUT_COLOR_ATTACHMENT;
    colorAttachment._finalLayout = ImageLayout::LAYOUT_PRESENT;

    AttachmentConfiguration &depthAttachment = opaquePassGenerator.MakeAttachment();
    depthAttachment._attachment._loadOp = AttachmentLoadOp::OP_CLEAR;
    depthAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    depthAttachment._renderTarget = _frameData[_frameIndex]._presentableSurface->GetDepthRenderTarget();
    depthAttachment._attachment._format = Format::FORMAT_D32_SFLOAT;
    depthAttachment._initialLayout = ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;
    depthAttachment._finalLayout = ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT;

    PushConstantConfiguration &mvpPushConstant = opaquePassGenerator.MakePushConstant();
    mvpPushConstant._pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    mvpPushConstant._pushConstant._size = sizeof(glm::mat4);
    mvpPushConstant._pushConstant._offset = 0;

    InputDescriptor vertexPosDataInputDescriptor {};
    vertexPosDataInputDescriptor._format = Format::FORMAT_R32G32B32_SFLOAT;
    vertexPosDataInputDescriptor._binding = 0;
    vertexPosDataInputDescriptor._location = 0;
    vertexPosDataInputDescriptor._memberOffset = offsetof(VertexData, position);
    vertexPosDataInputDescriptor._bEnabled = true;

    InputDescriptor colorDataInputDescriptor {};
    colorDataInputDescriptor._format = Format::FORMAT_R32G32B32_SFLOAT;
    colorDataInputDescriptor._binding = 0;
    colorDataInputDescriptor._location = 1;
    colorDataInputDescriptor._memberOffset = offsetof(VertexData, color);

    InputGroupDescriptor& vertexBufferInput = opaquePassGenerator.MakeInputGroupDescriptor();
    vertexBufferInput._stride = sizeof(VertexData);
    vertexBufferInput._inputDescriptors.emplace_back(vertexPosDataInputDescriptor);
    vertexBufferInput._inputDescriptors.emplace_back(colorDataInputDescriptor);

    const glm::mat4 projectionMatrix = glm::perspective(
        cameraFov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);
    
    glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, 90.f, glm::vec3(0.0f, 1.f, 0.0f));
    const glm::mat4 mvp_matrix = projectionMatrix * viewMatrix * model_matrix;
    mvpPushConstant._data = mvp_matrix;

    // Create primitive input descriptors
    GenerateSceneProxies(meshComponentPtr, &opaquePassGenerator);

    graphBuilder->AddPass(_renderContext, _frameData[_frameIndex]._commandPool.get(), opaquePassGenerator,_frameIndex);
}

// TODO WIP - not finalized
void RenderSystem::SetupFloorGridRenderPass(GraphBuilder* graphBuilder, const entt::registry& registry) {
    auto view = registry.view<const CameraComponent, const MeshComponent>();
    
    float cameraFov;
    glm::mat4 viewMatrix;
    const MeshComponent* meshComponentPtr = nullptr;
    for (auto entity : view) {
        auto [cameraComponent, meshComponent] = view.get<CameraComponent, MeshComponent>(entity);
        viewMatrix = cameraComponent.m_ViewMatrix;
        cameraFov = cameraComponent.m_Fov;
        meshComponentPtr = &meshComponent;
        break;
    }
    
    const glm::mat4 projectionMatrix = glm::perspective(
        cameraFov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);

    // Initialize geometry static, until we have a better way to create it
    static bool bFloorMeshInitialized = false;
    static MeshComponent meshComponent;
    if(!bFloorMeshInitialized) {
        const std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2};

        const std::vector<VertexData> vertexData = {
            {{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, // 0
            {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // 1
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // 2
            {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        };

        PrimitiveData primitive;
        primitive._indices = std::move(indices);
        primitive._vertexData = std::move(vertexData);
        primitive._vertexOffset = indices.size() * sizeof(unsigned int);
        
        MeshNode meshNode;
        meshNode._nodeName = "FloorGrid";
        meshNode._primitives.push_back(primitive);
        meshNode._buffer = std::make_shared<Buffer>(_renderContext);
        meshComponent._meshNodes.push_back(std::move(meshNode));
        
        graphBuilder->CopyGeometryData(meshNode._buffer, &meshNode);
        graphBuilder->UploadBufferData(meshNode._buffer, _frameData[_frameIndex]._commandPool->GetCommandBuffer().get());
    }
    
    RenderPassGenerator renderPassGenerator;
    
    ShaderConfiguration &vertexShader = renderPassGenerator.ConfigureShader(ShaderStage::STAGE_VERTEX);
    vertexShader._shaderPath = COMBINE_SHADER_DIR(floor_grid_vs.spv);

    ShaderConfiguration &fragmentShader = renderPassGenerator.ConfigureShader(ShaderStage::STAGE_FRAGMENT);
    fragmentShader._shaderPath = COMBINE_SHADER_DIR(floor_grid_fs.spv);

    RasterizationConfiguration &rasterizationConfig = renderPassGenerator.ConfigureRasterizationOptions();
    rasterizationConfig._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    rasterizationConfig._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;

    AttachmentConfiguration &colorAttachment = renderPassGenerator.MakeAttachment();
    colorAttachment._attachment._loadOp = AttachmentLoadOp::OP_LOAD;
    colorAttachment._attachment._storeOp = AttachmentStoreOp::OP_STORE;
    colorAttachment._attachment._stencilLoadOp = AttachmentLoadOp::OP_DONT_CARE;
    colorAttachment._attachment._stencilStoreOp = AttachmentStoreOp::OP_DONT_CARE;
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
    PushConstantConfiguration& viewMatrixPushConstant = renderPassGenerator.MakePushConstant();
    viewMatrixPushConstant._pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    viewMatrixPushConstant._pushConstant._size = sizeof(glm::mat4);
    viewMatrixPushConstant._pushConstant._offset = 0;
    viewMatrixPushConstant._data = viewMatrix;
    
    // projection matrix
    PushConstantConfiguration& projPushConstant = renderPassGenerator.MakePushConstant();
    projPushConstant._pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    projPushConstant._pushConstant._size = sizeof(glm::mat4);
    projPushConstant._pushConstant._offset = 0;
    projPushConstant._data = projectionMatrix;
    
    InputDescriptor vertexPosDataInputDescriptor {};
    vertexPosDataInputDescriptor._format = Format::FORMAT_R32G32B32_SFLOAT;
    vertexPosDataInputDescriptor._binding = 0;
    vertexPosDataInputDescriptor._location = 0;
    vertexPosDataInputDescriptor._memberOffset = offsetof(VertexData, position);
    vertexPosDataInputDescriptor._bEnabled = true;
    
    InputGroupDescriptor& vertexBufferInput = renderPassGenerator.MakeInputGroupDescriptor();
    vertexBufferInput._stride = sizeof(VertexData);
    vertexBufferInput._inputDescriptors.emplace_back(vertexPosDataInputDescriptor);
    
    GenerateSceneProxies(&meshComponent, &renderPassGenerator);

    graphBuilder->AddPass(_renderContext, _frameData[_frameIndex]._commandPool.get(), renderPassGenerator,_frameIndex);
}

void RenderSystem::GenerateSceneProxies(const MeshComponent* sceneComponent, RenderPassGenerator* renderPassGenerator) {
    if(sceneComponent && !sceneComponent->_meshNodes.empty() && renderPassGenerator) {
        for(const MeshNode& meshNode : sceneComponent->_meshNodes) {
            // All this offset logic could already be pre-computed when we load the geometry
            GeometryLoaderSystem::ForEachNodeConst(&meshNode, [&, this](const MeshNode* currentNode) {
                for(const PrimitiveData& primitiveData : meshNode._primitives) {
                    // This is not yet correct, i also need a way to filter primitives for passes
                    PrimitiveProxy& primitiveDataDescription = renderPassGenerator->MakePrimitiveProxy();
                    primitiveDataDescription._indicesCount = primitiveData._indices.size();
                    primitiveDataDescription._indicesBufferOffset = primitiveData._indicesOffset;
                    primitiveDataDescription._vOffset.push_back(primitiveData._vertexOffset);
                    primitiveDataDescription._primitiveBuffer = meshNode._buffer;
                }
            });
        }
    }
}
