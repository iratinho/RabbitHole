#include "Renderer/render_context.hpp"
#include "Renderer/RenderSystem.hpp"

#include <window.hpp>

#include "Core/GeometryLoaderSystem.hpp"
#include "Core/Components/CameraComponent.hpp"
#include "Core/Components/TransformComponent.hpp"
#include "Core/Components/UserInterfaceComponent.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/RenderPass/OpaqueRenderPass.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Surface.hpp"
#include "Renderer/RenderGraph/GraphBuilder.hpp"
#include "Renderer/RenderPass/FloorGridRenderPass.hpp"
#include "Renderer/RenderPass/FullScreenQuadRenderPass.hpp"

// TODO Lets create a command pool per frame instead per pass.. we need to have a command pool per swapchain image only

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
    
    render_context_ = new RenderContext(this);
    if(!render_context_->Initialize(initialization_params)) {
        return false;
    }
    
    frame_data_.resize(render_context_->GetSwapchain()->GetSwapchainImageCount());

    render_graph_ = new RenderGraph(render_context_);
    
    auto graph_builder = render_graph_->MakeGraphBuilder("GraphBuilder_For_Initialization");
    
    for (size_t i = 0; i < frame_data_.size(); ++i)
    {
        // Pre-Allocate sharedPtrs
        FrameData& frameData = frame_data_[i];
        frameData._commandPool = std::make_shared<CommandPool>(render_context_);
        frameData._presentableSurface = std::make_shared<Surface>();
        frameData.sync_primitives.in_flight_fence_new = std::make_shared<Fence>(render_context_);
        
        SurfaceCreateParams surfaceCreateParams;
        surfaceCreateParams._swapChainRenderTarget = render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::COLOR, i);
        surfaceCreateParams._swapChainRenderTargetDepth = render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::DEPTH, i);
        surfaceCreateParams._swapChain = render_context_->GetSwapchain();
        surfaceCreateParams._renderContext = render_context_;
        
        graph_builder.AllocateSurface(frameData._presentableSurface, surfaceCreateParams);
        graph_builder.AllocateFence(frameData.sync_primitives.in_flight_fence_new);
        graph_builder.AllocateCommandPool(frame_data_[i]._commandPool.get());
    }

    // Ask for the initial presentable image
    graph_builder.AcquirePresentableSurface(0);
    
    graph_builder.Execute();
    
    frame_idx = 0;

    // VALIDATE_RETURN(CreateRenderingResources());
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

    frame_idx = render_context_->GetSwapchain()->GetNextPresentableImage();
    
    const glm::mat4 projectionMatrix = glm::perspective(
        fov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);
    
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    modelMatrix  = glm::rotate(modelMatrix, 90.f, glm::vec3(0.0f, 1.f, 0.0f));
    
    // Graph builders are disposable and not cached per frame, we always create a new one
    auto graph_builder = render_graph_->MakeGraphBuilder(fmt::format("GraphBuilder_Swapchain_{0}", frame_idx));
    
    // Wait for the previous frame finish rendering
    graph_builder.WaitFence(frame_data_[frame_idx].sync_primitives.in_flight_fence_new.get());
    
    // Ask to presentation engine for a swapchain image
    graph_builder.AcquirePresentableSurface(frame_idx);
    
    // Clear current frame fences and command buffers from the pool
    graph_builder.ResetFence(frame_data_[frame_idx].sync_primitives.in_flight_fence_new.get());
    graph_builder.ResetCommandPool(frame_data_[frame_idx]._commandPool.get());

    // Grab a command buffer for this frame
    graph_builder.AllocateCommandBuffer(frame_data_[frame_idx]._commandPool.get());
    graph_builder.EnableCommandBufferRecording(frame_data_[frame_idx]._commandPool.get());

    // Allocate buffers for geometry that haven't been initialized yet
    AllocateGeometryBuffers(registry, &graph_builder, frame_idx);
    
    OpaquePassDesc opaquePassDesc {};
    opaquePassDesc.scene_color = frame_data_[frame_idx]._presentableSurface->GetRenderTarget();
    opaquePassDesc.scene_depth = frame_data_[frame_idx]._presentableSurface->GetDepthRenderTarget();
    opaquePassDesc.previousPassIndex = 0;
    opaquePassDesc.nextPassIndex = VK_SUBPASS_EXTERNAL;
    opaquePassDesc.frameIndex = (int)frame_idx;
    opaquePassDesc.viewMatrix = viewMatrix;
    opaquePassDesc.projectionMatrix = projectionMatrix;
    opaquePassDesc._commandPool = frame_data_[frame_idx]._commandPool.get();
    opaquePassDesc.meshNodes = &sceneComponentPtr->_meshNodes;
    graph_builder.MakePass<OpaquePassDesc>(&opaquePassDesc);
    
    FloorGridPassDesc floorGridPassDesc {};
    floorGridPassDesc.scene_color = frame_data_[frame_idx]._presentableSurface->GetRenderTarget();
    floorGridPassDesc.scene_depth = frame_data_[frame_idx]._presentableSurface->GetDepthRenderTarget();
    floorGridPassDesc.previousPassIndex = VK_SUBPASS_EXTERNAL;
    floorGridPassDesc.nextPassIndex = 0;
    floorGridPassDesc.enabled_ = true;
    floorGridPassDesc.frameIndex = (int)frame_idx;
    floorGridPassDesc.viewMatrix = viewMatrix;
    floorGridPassDesc.projectionMatrix = projectionMatrix;
    floorGridPassDesc._commandPool = frame_data_[frame_idx]._commandPool.get();
    graph_builder.MakePass<FloorGridPassDesc>(&floorGridPassDesc);
    
//    FullScreenQuadPassDesc fullScreenQuadDesc;
//    fullScreenQuadDesc.sceneColor = frame_data_[frame_idx]._presentableSurface->GetRenderTarget();
//    fullScreenQuadDesc.sceneDepth = frame_data_[frame_idx]._presentableSurface->GetDepthRenderTarget();
//    fullScreenQuadDesc.texture = uiRenderTarget;
//    fullScreenQuadDesc.frameIndex = (int)frame_idx;
//    fullScreenQuadDesc._commandPool = frame_data_[frame_idx]._commandPool.get();
//    graph_builder.MakePass<FullScreenQuadPassDesc>(&fullScreenQuadDesc);
    
    graph_builder.DisableCommandBufferRecording(frame_data_[frame_idx]._commandPool.get());

    SubmitCommandParams submitParams {};
    submitParams.waitSemaphore = render_context_->GetSwapchain()->GetSyncPrimtiive(frame_idx);
    submitParams.signalSemaphore = frame_data_[frame_idx].sync_primitives.render_finish_semaphore;
    submitParams.queueFence = frame_data_[frame_idx].sync_primitives.in_flight_fence_new.get();
    graph_builder.SubmitCommands(frame_data_[frame_idx]._commandPool.get(), submitParams);
    
    SurfacePresentParams presentParams {};
    presentParams._frameIndex = frame_idx;
    presentParams._waitSemaphore = frame_data_[frame_idx].sync_primitives.render_finish_semaphore;
    graph_builder.Present(frame_data_[frame_idx]._presentableSurface, presentParams);
    
    bool bWasSuccessfully = graph_builder.Execute();

    if(!bWasSuccessfully) {
        return false;
    }
    
    return true;
}

void RenderSystem::HandleResize(int width, int height)
{
    render_context_->MarkSwapchainDirty();
}

bool RenderSystem::CreateSyncPrimitives()
{
    if (render_context_)
    {
        VkResult result;

        for (size_t i = 0; i < frame_data_.size(); ++i)
        {
            SyncPrimitives sync_primitives;

            // Semaphore that will be signaled when when the first command buffer finish execution
            {
                VkSemaphoreCreateInfo command_buffer_finished_semaphore_create_info;
                command_buffer_finished_semaphore_create_info.flags = 0;
                command_buffer_finished_semaphore_create_info.pNext = nullptr;
                command_buffer_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                result = VkFunc::vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(),
                                           &command_buffer_finished_semaphore_create_info, nullptr,
                                           &sync_primitives.render_finish_semaphore);

                if (result != VK_SUCCESS)
                {
                    return false;
                }
            }
            
            frame_data_[i].sync_primitives.render_finish_semaphore = sync_primitives.render_finish_semaphore;
        }

        return true;
    }

    return false;
}

bool RenderSystem::ReleaseResources() {
    render_graph_->ReleaseRenderTargets();
    render_graph_->ReleasePassResources();
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
                    currentNode->_buffer = std::make_shared<Buffer>(render_context_);
                    graphBuilder->CopyGeometryData(currentNode->_buffer, currentNode);
                    graphBuilder->UploadBufferData(currentNode->_buffer, frame_data_[frameIndex]._commandPool->GetCommandBuffer().get());
                    currentNode->_bWasProcessed = true;
                }
            });
        }
    }
}
