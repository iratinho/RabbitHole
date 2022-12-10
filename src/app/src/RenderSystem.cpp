#include "RenderSystem.h"

#include <window.h>

#include "FloorGridRenderer.h"
#include "OpaqueRenderer.h"
#include "render_context.h"

namespace app::renderer {
    bool RenderSystem::Initialize(InitializationParams initialization_params) {
        render_context_ = new RenderContext();
        render_context_->Initialize(initialization_params);
        frame_data_.resize(render_context_->GetSwapchainImageCount());
        frame_idx = 0;
        
        renderers_.reserve(2);
        // floor_grid_renderer_ = new FloorGridRenderer();
        opaque_renderer_ = new OpaqueRenderer();

        // renderers_.push_back(floor_grid_renderer_);
        renderers_.push_back(opaque_renderer_);

        for (IRenderer* renderer : renderers_) {
            renderer->Initialize(render_context_, initialization_params);
        }
        
        VALIDATE_RETURN(CreateRenderingResources());
        VALIDATE_RETURN(CreateSyncPrimitives());

        return true;
    }

    bool RenderSystem::Process() {
        // Wait for the previous frame finish rendering
        vkWaitForFences(render_context_->GetLogicalDeviceHandle(), 1, &frame_data_[frame_idx].sync_primitives.in_flight_fence, VK_TRUE, UINT64_MAX);

        uint32_t swapchain_image_index;
        const VkResult result = vkAcquireNextImageKHR(render_context_->GetLogicalDeviceHandle(), render_context_->GetSwapchainHandle(), UINT64_MAX, frame_data_[frame_idx].sync_primitives.swapchain_image_semaphore, VK_NULL_HANDLE, &swapchain_image_index);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || needs_swapchain_recreation) {
            RecreateSwapchain();
            needs_swapchain_recreation = false;
            return false;
        }
        
        vkResetFences(render_context_->GetLogicalDeviceHandle(), 1, &frame_data_[frame_idx].sync_primitives.in_flight_fence);
        vkResetCommandPool(render_context_->GetLogicalDeviceHandle(), frame_data_[frame_idx].command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        std::vector<VkCommandBuffer> command_buffers;
        
        for (IRenderer* renderer : renderers_) {
            VkCommandBuffer command_buffer = renderer->RecordCommandBuffers(swapchain_image_index);
            command_buffers.push_back(command_buffer);
        }

        // Im not yet sure who should handle the submit info.. seems off
        VkSubmitInfo submit_info {};
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

        std::vector<unsigned> indices = { 1 };
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

        frame_idx += 1 % render_context_->GetSwapchainImageCount() - 1;
        
        return true;        
    }

    void RenderSystem::HandleResize(int width, int height) {
        needs_swapchain_recreation = true;
        invalid_surface_for_swapchain = false;

        // When we have a height or width of zero we will not be able to create a proper swapchain,
        // if that's the case mark it as invalid surface area, so that we dont even try to create the swapchain
        if(width == 0 || height == 0) {
            invalid_surface_for_swapchain = true;
        }
    }

    bool RenderSystem::CreateRenderingResources() {
        const int swapchain_image_count = render_context_->GetSwapchainImageCount();
        if(swapchain_image_count <= 0) {
            return false;
        }
        
        for (int i = 0; i < render_context_->GetSwapchainImageCount(); ++i) {
            VkCommandPoolCreateInfo command_pool_create_info;
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.queueFamilyIndex = render_context_->GetGraphicsQueueIndex();
        
            const VkResult result = vkCreateCommandPool(render_context_->GetLogicalDeviceHandle(), &command_pool_create_info, nullptr, &frame_data_[i].command_pool);
        
            if(result != VK_SUCCESS) {
                return false;
            }

            // Lets ask the renderers to create the command buffers and frame buffers
            for (IRenderer* renderer : renderers_) {
                renderer->AllocateCommandBuffers(frame_data_[i].command_pool, i);
                renderer->AllocateFrameBuffers(i);
            }
        }

        for (IRenderer* renderer : renderers_) {
            renderer->AllocateRenderingResources();
        }

        return true;
    }

    bool RenderSystem::CreateSyncPrimitives() {
        if(render_context_) {
            VkResult result;

            for (size_t i = 0; i < frame_data_.size(); ++i)
            {
                SyncPrimitives sync_primitives {};

                // Semaphore that will be signaled when the swapchain has an image ready
                {
                    VkSemaphoreCreateInfo acquire_semaphore_create_info;
                    acquire_semaphore_create_info.flags = 0;
                    acquire_semaphore_create_info.pNext = nullptr;
                    acquire_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                    result = vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(), &acquire_semaphore_create_info, nullptr, &sync_primitives.swapchain_image_semaphore);

                    if(result != VK_SUCCESS) {
                        return false;
                    }
                }

                // Semaphore that will be signaled when when the first command buffer finish execution
                {
                    VkSemaphoreCreateInfo command_buffer_finished_semaphore_create_info;
                    command_buffer_finished_semaphore_create_info.flags = 0;
                    command_buffer_finished_semaphore_create_info.pNext = nullptr;
                    command_buffer_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                    result = vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(), &command_buffer_finished_semaphore_create_info, nullptr, &sync_primitives.render_finish_semaphore);

                    if(result != VK_SUCCESS) {
                        return false;
                    }
                }

                // Fence that will block until queue commands finished executing
                {
                    VkFenceCreateInfo fence_create_info;
                    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                    fence_create_info.pNext = nullptr;
                    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    result = vkCreateFence(render_context_->GetLogicalDeviceHandle(), &fence_create_info, nullptr, &sync_primitives.in_flight_fence);

                    if(result != VK_SUCCESS) {
                        return false;
                    }
                }

                frame_data_[i].sync_primitives = sync_primitives;
            }

            return true;
        }

        return false;
    }

    bool RenderSystem::RecreateSwapchain() {
        while (invalid_surface_for_swapchain) {
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

        const VkExtent2D extent = render_context_->GetSwapchainExtent();
        for (IRenderer* renderer : renderers_) {
            renderer->HandleResize(extent.width, extent.height);
        }

        return false;
    }

}
