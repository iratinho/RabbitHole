#include "Renderer/RenderSystem.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/RenderPass/OpaqueRenderPass.hpp"
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

RenderSystem::~RenderSystem() {
    // TODO clear render graph
    // render_targets_.scene_color_render_target->FreeResource();
    // delete render_targets_.scene_color_render_target;
    // render_targets_.scene_depth_render_target->FreeResource();
    // delete render_targets_.scene_depth_render_target;
}

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
    const SceneComponent* sceneComponentPtr = nullptr;

    auto view = registry.view<const TransformComponent, const CameraComponent, const UserInterfaceComponent, const SceneComponent>();
    
    for (auto entity : view) {
        auto [transformComponent, cameraComponent, userInterfaceComponent, sceneComponent] = view.get<TransformComponent, CameraComponent, UserInterfaceComponent, SceneComponent>(entity);
        
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
    
    OpaquePassDesc opaquePassDesc {};
    opaquePassDesc.scene_color = _frameData[_frameIndex]._presentableSurface->GetRenderTarget();
    opaquePassDesc.scene_depth = _frameData[_frameIndex]._presentableSurface->GetDepthRenderTarget();
    opaquePassDesc.previousPassIndex = 0;
    opaquePassDesc.nextPassIndex = VK_SUBPASS_EXTERNAL;
    opaquePassDesc.frameIndex = (int)_frameIndex;
    opaquePassDesc.viewMatrix = viewMatrix;
    opaquePassDesc.projectionMatrix = projectionMatrix;
    opaquePassDesc._commandPool = _frameData[_frameIndex]._commandPool.get();
    opaquePassDesc.meshNodes = &sceneComponentPtr->_meshNodes;
    graphBuilder.MakePass<OpaquePassDesc>(&opaquePassDesc);
    
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
    SceneComponent* sceneComponent = nullptr;

    for (auto view = registry.view<SceneComponent>(); auto entity : view) {
        sceneComponent = const_cast<SceneComponent*>(&view.get<SceneComponent>(entity));
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
