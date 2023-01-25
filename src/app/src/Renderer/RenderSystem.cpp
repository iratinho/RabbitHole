#include "window.h"
#include "render_context.h"
#include "RenderSystem.h"
#include "RenderTarget.h"
#include "RenderGraph/GraphBuilder.h"
#include "OpaqueRenderPass.h"

RenderSystem::~RenderSystem() {
    // TODO clear render graph
    // render_targets_.scene_color_render_target->FreeResource();
    // delete render_targets_.scene_color_render_target;
    // render_targets_.scene_depth_render_target->FreeResource();
    // delete render_targets_.scene_depth_render_target;
}

bool RenderSystem::Initialize(InitializationParams initialization_params)
{
    render_context_ = new RenderContext();
    render_context_->Initialize(initialization_params);
    frame_data_.resize(render_context_->GetSwapchainImageCount());
    frame_idx = 0;

    render_graph_ = new RenderGraph(render_context_);

    VALIDATE_RETURN(CreateSwapchainRenderTargets());
    VALIDATE_RETURN(CreateRenderingResources());
    VALIDATE_RETURN(CreateSyncPrimitives());
    
    return true;
}

bool RenderSystem::Process() {
    // Wait for the previous frame finish rendering
    vkWaitForFences(render_context_->GetLogicalDeviceHandle(), 1, &frame_data_[frame_idx].sync_primitives.in_flight_fence, VK_TRUE, UINT64_MAX);

    uint32_t swapchain_image_index;
    const VkResult result = vkAcquireNextImageKHR(render_context_->GetLogicalDeviceHandle(), render_context_->GetSwapchainHandle(), UINT64_MAX, frame_data_[frame_idx].sync_primitives.swapchain_image_semaphore, VK_NULL_HANDLE, &swapchain_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || needs_swapchain_recreation) {
        RecreateSwapchain();
        needs_swapchain_recreation = false;
        return false;
    }

    vkResetFences(render_context_->GetLogicalDeviceHandle(), 1, &frame_data_[frame_idx].sync_primitives.in_flight_fence);
    vkResetCommandPool(render_context_->GetLogicalDeviceHandle(), frame_data_[frame_idx].command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

    // Graph builders are disposable and not cached per frame, we always create a new one
    auto graph_builder = render_graph_->MakeGraphBuilder(std::format("GraphBuilder_Swapchain_{0}", swapchain_image_index));

    OpaquePassDesc desc {};
    desc.scene_color = std::format("$.scene_color_{0}", swapchain_image_index);
    desc.scene_depth = std::format("$.scene_depth_{0}", swapchain_image_index);
    desc.enabled_ = true;
    graph_builder.MakePass<OpaquePassDesc>(desc);

    graph_builder.Execute();
    
    auto command_buffers = graph_builder.GetCommandBuffers();
    
    // Im not yet sure who should handle the submit info.. seems off
    VkSubmitInfo submit_info{};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pNext = nullptr;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = command_buffers.size();
    submit_info.pCommandBuffers = command_buffers.data();
    submit_info.pWaitSemaphores = &frame_data_[frame_idx].sync_primitives.swapchain_image_semaphore;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = waitStages;
    submit_info.pSignalSemaphores = &frame_data_[frame_idx].sync_primitives.render_finish_semaphore;
    submit_info.signalSemaphoreCount = 1;
    vkQueueSubmit(render_context_->GetGraphicsQueueHandle(), 1, &submit_info, frame_data_[frame_idx].sync_primitives.in_flight_fence);

    std::vector<unsigned> indices = {1};
    const VkSwapchainKHR swapchain = render_context_->GetSwapchainHandle();
    VkPresentInfoKHR present_info_khr;
    present_info_khr.pNext = nullptr;
    present_info_khr.pResults = nullptr;
    present_info_khr.pSwapchains = &swapchain;
    present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info_khr.swapchainCount = 1;
    present_info_khr.pImageIndices = &swapchain_image_index;
    present_info_khr.pWaitSemaphores = &frame_data_[frame_idx].sync_primitives.render_finish_semaphore;
    present_info_khr.waitSemaphoreCount = 1;
    vkQueuePresentKHR(render_context_->GetPresentQueueHandle(), &present_info_khr);

    frame_idx +=  1 % render_context_->GetSwapchainImageCount() - 1;

    return true;
}

void RenderSystem::HandleResize(int width, int height)
{
    needs_swapchain_recreation = true;
    invalid_surface_for_swapchain = false;

    // When we have a height or width of zero we will not be able to create a proper swapchain,
    // if that's the case mark it as invalid surface area, so that we dont even try to create the swapchain
    if (width == 0 || height == 0)
    {
        invalid_surface_for_swapchain = true;
    }
}

bool RenderSystem::CreateSwapchainRenderTargets()
{
    // Create the swapchain render targets and cache them in the render graph
    for (int i = 0; i < render_context_->GetSwapchainImageCount(); ++i)
    {
        TextureParams color_texture_params;
        color_texture_params.format = VK_FORMAT_B8G8R8A8_SRGB;
        color_texture_params.height = render_context_->GetSwapchainExtent().height;
        color_texture_params.width = render_context_->GetSwapchainExtent().width;
        color_texture_params.sample_count = 0;

        Texture color_texture = Texture(render_context_, color_texture_params, render_context_->GetSwapchainImages()[i]);
        const auto color_depth_render_target = new RenderTarget(std::move(color_texture));

        if(!color_depth_render_target->Initialize())
        {
            return false;
        }
        
        TextureParams depth_color_params;
        depth_color_params.format = VK_FORMAT_D32_SFLOAT;
        depth_color_params.sample_count = 0;
        depth_color_params.width = render_context_->GetSwapchainExtent().width;
        depth_color_params.height = render_context_->GetSwapchainExtent().height;
        const auto scene_depth_render_target = new RenderTarget(render_context_, depth_color_params);

        if(!scene_depth_render_target->Initialize())
        {
            return false;
        }

        render_graph_->RegisterRenderTarget(std::format("$.scene_color_{0}", i), color_depth_render_target);
        render_graph_->RegisterRenderTarget(std::format("$.scene_depth_{0}", i), scene_depth_render_target);
    }

    return true;
}

bool RenderSystem::CreateRenderingResources()
{
    const int swapchain_image_count = render_context_->GetSwapchainImageCount();
    if (swapchain_image_count <= 0)
    {
        return false;
    }

    for (int i = 0; i < render_context_->GetSwapchainImageCount(); ++i)
    {
        VkCommandPoolCreateInfo command_pool_create_info;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = render_context_->GetGraphicsQueueIndex();

        const VkResult result = vkCreateCommandPool(render_context_->GetLogicalDeviceHandle(),
                                                    &command_pool_create_info, nullptr, &frame_data_[i].command_pool);

        if (result != VK_SUCCESS)
        {
            return false;
        }
    }

    return true;
}

bool RenderSystem::CreateSyncPrimitives()
{
    if (render_context_)
    {
        VkResult result;

        for (size_t i = 0; i < frame_data_.size(); ++i)
        {
            SyncPrimitives sync_primitives{};

            // Semaphore that will be signaled when the swapchain has an image ready
            {
                VkSemaphoreCreateInfo acquire_semaphore_create_info;
                acquire_semaphore_create_info.flags = 0;
                acquire_semaphore_create_info.pNext = nullptr;
                acquire_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                result = vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(), &acquire_semaphore_create_info,
                                           nullptr, &sync_primitives.swapchain_image_semaphore);

                if (result != VK_SUCCESS)
                {
                    return false;
                }
            }

            // Semaphore that will be signaled when when the first command buffer finish execution
            {
                VkSemaphoreCreateInfo command_buffer_finished_semaphore_create_info;
                command_buffer_finished_semaphore_create_info.flags = 0;
                command_buffer_finished_semaphore_create_info.pNext = nullptr;
                command_buffer_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                result = vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(),
                                           &command_buffer_finished_semaphore_create_info, nullptr,
                                           &sync_primitives.render_finish_semaphore);

                if (result != VK_SUCCESS)
                {
                    return false;
                }
            }

            // Fence that will block until queue commands finished executing
            {
                VkFenceCreateInfo fence_create_info;
                fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                fence_create_info.pNext = nullptr;
                fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                result = vkCreateFence(render_context_->GetLogicalDeviceHandle(), &fence_create_info, nullptr,
                                       &sync_primitives.in_flight_fence);

                if (result != VK_SUCCESS)
                {
                    return false;
                }
            }

            frame_data_[i].sync_primitives = sync_primitives;
        }

        return true;
    }

    return false;
}

bool RenderSystem::RecreateSwapchain()
{
    while (invalid_surface_for_swapchain)
    {
        const VkExtent2D extent = render_context_->GetSwapchainExtent();
        invalid_surface_for_swapchain = extent.width == 0 || extent.height == 0;

        /** Since at the moment the application is single threaded, if we are inside of this loop we will not be
        * able to pool events from the window, so if we are minimized we will not be able to recover from it
        * unless we keep pooling events **/
        render_context_->GetWindow()->PoolEvents();
    }
    
    // Wait until all operations are completed
    vkDeviceWaitIdle(render_context_->GetLogicalDeviceHandle());
    
    render_context_->RecreateSwapchain();
    
    render_graph_->ReleaseRenderTargets();
    render_graph_->ReleasePassResources();

    CreateSwapchainRenderTargets();
    
    return true;
};
