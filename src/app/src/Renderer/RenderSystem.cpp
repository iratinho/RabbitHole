#include "Renderer/render_context.h"
#include "Renderer/RenderSystem.h"

#include <window.h>

#include "Core/Components/CameraComponent.h"
#include "Core/Components/TransformComponent.h"
#include "Core/Components/UserInterfaceComponent.h"
#include "Renderer/VulkanLoader.h"
#include "Renderer/RenderPass/OpaqueRenderPass.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/RenderGraph/GraphBuilder.h"
#include "Renderer/RenderPass/FloorGridRenderPass.h"
#include "Renderer/RenderPass/FullScreenQuadRenderPass.h"

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
    frame_idx = 0;

    render_graph_ = new RenderGraph(render_context_);

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

    auto view = registry.view<const TransformComponent, const CameraComponent, const UserInterfaceComponent>();

    for (auto entity : view) {
        auto [transformComponent, cameraComponent, userInterfaceComponent] = view.get<TransformComponent, CameraComponent, UserInterfaceComponent>(entity);
        
        cameraPosition = transformComponent.m_Position;
        viewMatrix = cameraComponent.m_ViewMatrix;
        fov = cameraComponent.m_Fov;
        uiRenderTarget = userInterfaceComponent._uiRenderTarget;
        
        break;
    }
    
    const glm::mat4 projectionMatrixFloor = glm::perspective(
        fov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.01f, 400.f);

    const glm::mat4 projectionMatrix = glm::perspective(
        fov, ((float)m_InitializationParams.window_->GetFramebufferSize().width / (float)m_InitializationParams.window_->GetFramebufferSize().height), 0.1f, 180.f);
    
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    modelMatrix  = glm::rotate(modelMatrix, 90.f, glm::vec3(0.0f, 1.f, 0.0f));
    
    // Graph builders are disposable and not cached per frame, we always create a new one
    auto graph_builder = render_graph_->MakeGraphBuilder(fmt::format("GraphBuilder_Swapchain_{0}", frame_idx));
    
    // Wait for the previous frame finish rendering
    graph_builder.WaitFence(frame_data_[frame_idx].sync_primitives.in_flight_fence_new.get());
    
    // Ask to presentation engine for a swapchain image
    graph_builder.AcquireSwapchainImage(frame_idx);

    // Grab a command buffer for this frame
    graph_builder.AllocateCommandBuffer(frame_idx);
    
    // Clear current frame fences and command buffers from the pool
    graph_builder.ResetFence(frame_data_[frame_idx].sync_primitives.in_flight_fence_new.get());
    graph_builder.ResetCommandPoolLambda([this]() -> VkCommandPool { return render_graph_->GetCommandBufferManager()->GetCommandBufferPool(render_graph_->GetCommandBufferManager()->GetCommandBuffer((int)frame_idx)); } );
    
    graph_builder.EnableCommandBufferRecording(frame_idx);
    
    FloorGridPassDesc floor_grid_pass_desc {};
    floor_grid_pass_desc.scene_color = [this]()-> RenderTarget* { return render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::COLOR, render_context_->GetSwapchain()->GetNextPresentableImage()); };
    floor_grid_pass_desc.scene_depth = [this]()-> RenderTarget* { return render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::DEPTH, render_context_->GetSwapchain()->GetNextPresentableImage()); };
    floor_grid_pass_desc.previousPassIndex = VK_SUBPASS_EXTERNAL;
    floor_grid_pass_desc.nextPassIndex = 0;
    floor_grid_pass_desc.enabled_ = true;
    floor_grid_pass_desc.frameIndex = (int)frame_idx;
    floor_grid_pass_desc.viewMatrix = viewMatrix;
    floor_grid_pass_desc.projectionMatrix = projectionMatrixFloor;
    graph_builder.MakePass<FloorGridPassDesc>(&floor_grid_pass_desc);
    
    OpaquePassDesc desc {};
    desc.scene_color = [this]()-> RenderTarget* { return render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::COLOR, render_context_->GetSwapchain()->GetNextPresentableImage()); };
    desc.scene_depth = [this]()-> RenderTarget* { return render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::DEPTH, render_context_->GetSwapchain()->GetNextPresentableImage()); };
    desc.previousPassIndex = 0;
    desc.nextPassIndex = VK_SUBPASS_EXTERNAL;
    desc.enabled_ = true;
    desc.frameIndex = (int)frame_idx;
    desc.viewMatrix = viewMatrix;
    desc.projectionMatrix = projectionMatrix;
    graph_builder.MakePass<OpaquePassDesc>(&desc);
    
    FullScreenQuadPassDesc fullScreenQuadDesc;
    fullScreenQuadDesc.sceneColor = [this]()-> RenderTarget* { return render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::COLOR, render_context_->GetSwapchain()->GetNextPresentableImage()); };
    fullScreenQuadDesc.sceneDepth = [this]()-> RenderTarget* { return render_context_->GetSwapchain()->GetSwapchainRenderTarget(ISwapchain::DEPTH, render_context_->GetSwapchain()->GetNextPresentableImage()); };
    fullScreenQuadDesc.texture = [&]() -> std::shared_ptr<RenderTarget> { return uiRenderTarget; };
    fullScreenQuadDesc.frameIndex = (int)frame_idx;
    graph_builder.MakePass<FullScreenQuadPassDesc>(&fullScreenQuadDesc);
    
    graph_builder.DisableCommandBufferRecording(frame_idx);
    
    bool bWasSuccessfully = graph_builder.Execute();

    if(!bWasSuccessfully) {
        return false;
    }
    
    VkCommandBuffer commandBuffer = render_graph_->GetCommandBufferManager()->GetCommandBuffer((int)frame_idx);
    VkSemaphore swapchainSemaphroe = render_context_->GetSwapchain()->GetSyncPrimtiive(frame_idx);
    
    // TODO create graph action for this
    VkSubmitInfo submit_info{};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pNext = nullptr;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer;
    submit_info.pWaitSemaphores = &swapchainSemaphroe;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = waitStages;
    submit_info.pSignalSemaphores = &frame_data_[frame_idx].sync_primitives.render_finish_semaphore;
    submit_info.signalSemaphoreCount = 1;
    VkFunc::vkQueueSubmit(render_context_->GetGraphicsQueueHandle(), 1, &submit_info, frame_data_[frame_idx].sync_primitives.in_flight_fence_new->GetResource());

    // Temporary should all be part of graph builder
    render_graph_->GetCommandBufferManager()->ReleaseCommandBuffer((int)frame_idx);
    
    const uint32_t imageIndex = render_context_->GetSwapchain()->GetNextPresentableImage();
    
    std::vector<unsigned> indices = {1};
    const VkSwapchainKHR swapchain = static_cast<VkSwapchainKHR>(render_context_->GetSwapchain()->GetNativeHandle());

    // TODO create graph action for this
    VkPresentInfoKHR present_info_khr;
    present_info_khr.pNext = nullptr;
    present_info_khr.pResults = nullptr;
    present_info_khr.pSwapchains = &swapchain;
    present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info_khr.swapchainCount = 1;
    present_info_khr.pImageIndices = &imageIndex;
    present_info_khr.pWaitSemaphores = &frame_data_[frame_idx].sync_primitives.render_finish_semaphore;
    present_info_khr.waitSemaphoreCount = 1;
    VkFunc::vkQueuePresentKHR(render_context_->GetPresentQueueHandle(), &present_info_khr);

    
    frame_idx = (frame_idx + 1) % render_context_->GetSwapchain()->GetSwapchainImageCount();
    
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
            sync_primitives.in_flight_fence_new = std::make_unique<Fence>(render_context_);

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

            // Fence that will block until queue commands finished executing
            {
                bool result2 = sync_primitives.in_flight_fence_new->Initialize();

                if (!result2) {
                    return false;
                }
            }

            frame_data_[i].sync_primitives = std::move(sync_primitives);
        }

        return true;
    }

    return false;
}

bool RenderSystem::ReleaseResources()
{
    render_graph_->ReleaseRenderTargets();
    render_graph_->ReleasePassResources();
    return true;
};
