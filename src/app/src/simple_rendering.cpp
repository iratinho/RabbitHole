#include "simple_rendering.h"
#include "window.h"
#include "render_context.h"

#define VALIDATE_RETURN(op) if(!op) return false

namespace app::renderer {
    bool SimpleRendering::Initialize(const InitializationParams& initialization_params) {
        render_context_ = new RenderContext;
        if(render_context_->Initialize(initialization_params)) {
            VALIDATE_RETURN(CreateRenderPass());
            VALIDATE_RETURN(CreateGraphicsPipeline());
            VALIDATE_RETURN(CreateSwapchainFramebuffers());
            VALIDATE_RETURN(CreateCommandPool());
            VALIDATE_RETURN(CreateCommandBuffers());
            VALIDATE_RETURN(CreateSyncObjects());

            window_ = initialization_params.window_;
            return true;
        }

        return false;
    }

    bool SimpleRendering::Draw() {
        // Wait for the previous frame finish rendering
        vkWaitForFences(render_context_->GetLogicalDeviceHandle(), 1, &syncrhonization_primitive_[current_frame_index].in_flight_fence, VK_TRUE, UINT64_MAX);

        uint32_t swapchain_image_index;
        const VkResult result = vkAcquireNextImageKHR(render_context_->GetLogicalDeviceHandle(), render_context_->GetSwapchainHandle(), UINT64_MAX, syncrhonization_primitive_[current_frame_index].swapchain_image_semaphore, VK_NULL_HANDLE, &swapchain_image_index);
        
        if(result == VK_ERROR_OUT_OF_DATE_KHR || needs_swapchain_recreation) {
            RecreateSwapchain();
            needs_swapchain_recreation = false;
            return false;
        }
        
        // Reset fence
        vkResetFences(render_context_->GetLogicalDeviceHandle(), 1, &syncrhonization_primitive_[current_frame_index].in_flight_fence);
        
        // Invalidate current command buffer to start new draw frame
        vkResetCommandBuffer(command_buffers_[current_frame_index], 0);

        // Records the commands use for drawing 
        RecordCommandBuffers(swapchain_framebuffers[swapchain_image_index]);

        // Submits the command buffers
        {
            VkSubmitInfo submit_info {};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            submit_info.pNext = nullptr;
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffers_[current_frame_index];
            submit_info.pWaitSemaphores = &syncrhonization_primitive_[current_frame_index].swapchain_image_semaphore;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitDstStageMask = waitStages;
            submit_info.pSignalSemaphores = &syncrhonization_primitive_[current_frame_index].render_finish_semaphore;
            submit_info.signalSemaphoreCount = 1;
            vkQueueSubmit(render_context_->GetGraphicsQueueHandle(), 1, &submit_info, syncrhonization_primitive_[current_frame_index].in_flight_fence);
        }

        // Present
        {
            std::vector<unsigned> indices = { 1 };
            VkSwapchainKHR swapchain = render_context_->GetSwapchainHandle();
            VkPresentInfoKHR present_info_khr;
            present_info_khr.pNext = nullptr;
            present_info_khr.pResults = nullptr;
            present_info_khr.pSwapchains = &swapchain;
            present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info_khr.swapchainCount = 1;
            present_info_khr.pImageIndices = &swapchain_image_index;
            present_info_khr.pWaitSemaphores = &syncrhonization_primitive_[current_frame_index].render_finish_semaphore;
            present_info_khr.waitSemaphoreCount = 1;
            
            vkQueuePresentKHR(render_context_->GetPresentQueueHandle(), &present_info_khr);
        }

        // Advance frame
        current_frame_index += 1 % render_context_->GetSwapchainImageCount() - 1;
        
        return true;
    }

    bool SimpleRendering::RecreateSwapchain() {
        // TODO dont like this, im freezing everything. What if we want to keep processing other parts of the application while minimized?
        while (invalid_surface_for_swapchain) {
            const VkExtent2D extent = render_context_->GetSwapchainExtent();
            invalid_surface_for_swapchain = extent.width == 0 || extent.height == 0;

            /** Since at the moment the application is single threaded, if we are inside of this loop we will not be
            * able to pool events from the window, so if we are minimized we will not be able to recover from it
            * unless we keep pooling events **/
            window_->PoolEvents();
        }
        
        // Wait until all operations are completed
        vkDeviceWaitIdle(render_context_->GetLogicalDeviceHandle());
        
        // Cleanup allocated framebuffer's
        for (auto framebuffer : swapchain_framebuffers) {
            vkDestroyFramebuffer(render_context_->GetLogicalDeviceHandle(), framebuffer, nullptr);
        }

        swapchain_framebuffers.clear();
        
        render_context_->RecreateSwapchain();
        CreateSwapchainFramebuffers();

        return true;
    }

    void SimpleRendering::HandleResize(int width, int height) {
        needs_swapchain_recreation = true;
        invalid_surface_for_swapchain = false;

        // When we have a height or width of zero we will not be able to create a proper swapchain,
        // if that's the case mark it as invalid surface area, so that we dont even try to create the swapchain
        if(width == 0 || height == 0) {
            invalid_surface_for_swapchain = true;
        }
    }

    bool SimpleRendering::CreateRenderPass() {
        VkAttachmentDescription color_attachment_description;
        color_attachment_description.flags = 0;
        color_attachment_description.format = VK_FORMAT_B8G8R8A8_SRGB; // hardcoded for now, we need to ask swapchain instead
        color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_reference;
        color_attachment_reference.attachment = 0; // matches to the render pass pAttachments array index
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass_description {};
        subpass_description.colorAttachmentCount = 1;
        subpass_description.pColorAttachments = &color_attachment_reference;
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            
        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            
        VkRenderPassCreateInfo render_pass_create_info {};
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.dependencyCount = 1; // investigate this
        render_pass_create_info.pAttachments = &color_attachment_description;
        render_pass_create_info.pDependencies = &subpass_dependency;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.subpassCount = 1;

        const VkResult result = vkCreateRenderPass(render_context_->GetLogicalDeviceHandle(), &render_pass_create_info, nullptr, &render_pass_);

        return result == VK_SUCCESS;
    }

    bool SimpleRendering::CreateGraphicsPipeline() {
        VkPipelineShaderStageCreateInfo vs_shader_stage {};
        VkPipelineShaderStageCreateInfo fs_shader_stage {};
        
        if(render_context_) {
            bool vs_shader_create = false;
            bool fs_shader_create = false;

            vs_shader_create = render_context_->CreateShader(R"(C:\dev\RabbitHole\src\app\shaders\bytecode\dummy_vs.spv)", VK_SHADER_STAGE_VERTEX_BIT, vs_shader_stage);
            fs_shader_create = render_context_->CreateShader(R"(C:\dev\RabbitHole\src\app\shaders\bytecode\dummy_fs.spv)", VK_SHADER_STAGE_FRAGMENT_BIT, fs_shader_stage);

            if(!vs_shader_create || !fs_shader_create) {
                return true;
            }

            std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {vs_shader_stage, fs_shader_stage};

            // https://vulkan.lunarg.com/doc/view/1.2.154.1/windows/tutorial/html/08-init_pipeline_layout.html
            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
            descriptor_set_layout_create_info.flags = 0;
            descriptor_set_layout_create_info.bindingCount = 0;
            descriptor_set_layout_create_info.pBindings = nullptr; // this is a VkDescriptorSetLayoutBinding for each stage (for uniforms)
            descriptor_set_layout_create_info.pNext = nullptr;
            descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            
            VkDescriptorSetLayout descriptor_set_layout;
            vkCreateDescriptorSetLayout(render_context_->GetLogicalDeviceHandle(), &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);

            VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.pNext = nullptr;
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            
            VkPipelineLayout pipeline_layout;
            vkCreatePipelineLayout(render_context_->GetLogicalDeviceHandle(), &pipeline_layout_create_info, nullptr, &pipeline_layout);

            std::array<VkDynamicState, 2> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            
            VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info;
            pipeline_dynamic_state_create_info.flags = 0;
            pipeline_dynamic_state_create_info.pNext = nullptr;
            pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
            pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states.data();

            VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info;
            pipeline_rasterization_state_create_info.flags = 0;
            pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            pipeline_rasterization_state_create_info.lineWidth = 1.0f;
            pipeline_rasterization_state_create_info.pNext = nullptr;
            pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
            pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
            pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info;
            pipeline_viewport_state_create_info.flags = 0;
            pipeline_viewport_state_create_info.pNext = nullptr;
            pipeline_viewport_state_create_info.pScissors = nullptr;
            pipeline_viewport_state_create_info.pViewports = nullptr;
            pipeline_viewport_state_create_info.scissorCount = 1;
            pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            pipeline_viewport_state_create_info.viewportCount = 1;

            // todo review color blending later
            VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state;
            pipeline_color_blend_attachment_state.blendEnable = VK_TRUE;
            pipeline_color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
            pipeline_color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
            pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            pipeline_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            pipeline_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            pipeline_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            pipeline_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            
            VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info;
            pipeline_color_blend_state_create_info.flags = 0;
            pipeline_color_blend_state_create_info.attachmentCount = 1;
            pipeline_color_blend_state_create_info.blendConstants[0] = 0;
            pipeline_color_blend_state_create_info.blendConstants[1] = 0;
            pipeline_color_blend_state_create_info.blendConstants[2] = 0;
            pipeline_color_blend_state_create_info.blendConstants[3] = 0;
            pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            pipeline_color_blend_state_create_info.pAttachments = &pipeline_color_blend_attachment_state;
            pipeline_color_blend_state_create_info.pNext = nullptr;
            pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;

            VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info;
            pipeline_input_assembly_state_create_info.flags = 0;
            pipeline_input_assembly_state_create_info.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            pipeline_input_assembly_state_create_info.pNext = nullptr;
            pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE; //  what is this??

            VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info;
            pipeline_vertex_input_state_create_info.flags = 0;
            pipeline_vertex_input_state_create_info.pNext = nullptr;
            pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            // We have no data for this state, since we build the geometry in the shader itself
            pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = nullptr;
            pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = nullptr;
            pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
            pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
            
            // For every subpass we must have pipeline unless they are compatible
            VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {};
            graphics_pipeline_create_info.flags = 0;
            graphics_pipeline_create_info.layout = pipeline_layout;
            graphics_pipeline_create_info.subpass = 0;
            graphics_pipeline_create_info.pStages = shader_stages.data();
            graphics_pipeline_create_info.renderPass = render_pass_;
            graphics_pipeline_create_info.stageCount = shader_stages.size();
            graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
            graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
            graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
            graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
            graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
            graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
            graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
            
            uint32_t pipeline_count = 1;
            std::vector<VkPipeline> pipelines(pipeline_count);
            vkCreateGraphicsPipelines(render_context_->GetLogicalDeviceHandle(), nullptr, pipeline_count, &graphics_pipeline_create_info, nullptr, pipelines.data());

            pipelines_ = pipelines;
        }
        
        return !pipelines_.empty();
    }

    // TODO move this to render context
    bool SimpleRendering::CreateSwapchainFramebuffers() {
        const int swapchain_image_count = render_context_->GetSwapchainImageCount();
        const VkExtent2D swapchain_extent = render_context_->GetSwapchainExtent();
        const std::vector<VkImageView>& swapchain_image_views = render_context_->GetSwapchainImageVies();
        
        for (int i = 0; i < swapchain_image_count; ++i) {
            VkFramebufferCreateInfo framebuffer_create_info;
            framebuffer_create_info.flags = 0;
            framebuffer_create_info.height = swapchain_extent.height; 
            framebuffer_create_info.layers = 1;
            framebuffer_create_info.width = swapchain_extent.width;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = &swapchain_image_views[i];
            framebuffer_create_info.pNext = nullptr;
            framebuffer_create_info.renderPass = render_pass_;
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

            VkFramebuffer framebuffer;
            VkResult result = vkCreateFramebuffer(render_context_->GetLogicalDeviceHandle(), &framebuffer_create_info, nullptr, &framebuffer);

            if(result == VK_SUCCESS) {
                swapchain_framebuffers.push_back(framebuffer);
            }
        }

        return !swapchain_framebuffers.empty();
    }

    bool SimpleRendering::CreateCommandPool() {
        VkCommandPoolCreateInfo command_pool_create_info;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = render_context_->GetGraphicsQueueIndex();
        
        const VkResult result = vkCreateCommandPool(render_context_->GetLogicalDeviceHandle(), &command_pool_create_info, nullptr, &command_pool_);
        
        return result == VK_SUCCESS;   
    }
    
    bool SimpleRendering::CreateCommandBuffers() {
        command_buffers_.resize(render_context_->GetSwapchainImageCount());
        
        for (int i = 0; i < render_context_->GetSwapchainImageCount(); ++i)
        {
            VkCommandBufferAllocateInfo command_buffer_allocate_info;
            command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            command_buffer_allocate_info.commandPool = command_pool_;
            command_buffer_allocate_info.pNext = nullptr;
            command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.commandBufferCount = 1;

            const VkResult result = vkAllocateCommandBuffers(render_context_->GetLogicalDeviceHandle(), &command_buffer_allocate_info, &command_buffers_[i]);

            if(result != VK_SUCCESS)
                return false;
        }

        return true;
    }

    bool SimpleRendering::RecordCommandBuffers(VkFramebuffer target_swapchain_framebuffer) {
        VkCommandBufferBeginInfo command_buffer_begin_info;
        command_buffer_begin_info.flags = 0;
        command_buffer_begin_info.pNext = nullptr;
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.pInheritanceInfo = nullptr;
        
        vkBeginCommandBuffer(command_buffers_[current_frame_index], &command_buffer_begin_info);

        // Render pass
        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.framebuffer = target_swapchain_framebuffer;
        render_pass_begin_info.pNext = nullptr;
        render_pass_begin_info.renderArea.extent = render_context_->GetSwapchainExtent();
        render_pass_begin_info.renderArea.offset = {0, 0 };
        render_pass_begin_info.renderPass = render_pass_;
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_color;
        
        vkCmdBeginRenderPass(command_buffers_[current_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        // Bind to graphics pipeline
        vkCmdBindPipeline(command_buffers_[current_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[0]);

        // Handle dynamic states of the pipeline
        {
            // Viewport
            VkViewport viewport;
            viewport.height = (float)render_context_->GetSwapchainExtent().height;
            viewport.width = (float)render_context_->GetSwapchainExtent().width;
            viewport.x = 0;
            viewport.y = 0;
            viewport.maxDepth = 1;
            viewport.minDepth = 0;
            
            vkCmdSetViewport(command_buffers_[current_frame_index], 0 , 1, &viewport);

            // Scissor
            VkRect2D scissor;
            scissor.offset = {0, 0};
            scissor.extent = render_context_->GetSwapchainExtent();

            vkCmdSetScissor(command_buffers_[current_frame_index], 0, 1, &scissor);
        }

        // Issue Draw command
        vkCmdDraw(command_buffers_[current_frame_index], 3, 1, 0, 0);
        
        vkCmdEndRenderPass(command_buffers_[current_frame_index]);
        
        const VkResult result = vkEndCommandBuffer(command_buffers_[current_frame_index]);

        return result == VK_SUCCESS;
    }

    bool SimpleRendering::CreateSyncObjects() {
        syncrhonization_primitive_.resize(render_context_->GetSwapchainImageCount());
        
        for (int i = 0; i < render_context_->GetSwapchainImageCount(); ++i) {
            SyncPrimitives sync_primitives {};
            
            // Semaphore that will be signaled when the swapchain has an image ready
            {
                VkSemaphoreCreateInfo acquire_semaphore_create_info;
                acquire_semaphore_create_info.flags = 0;
                acquire_semaphore_create_info.pNext = nullptr;
                acquire_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(), &acquire_semaphore_create_info, nullptr, &sync_primitives.swapchain_image_semaphore);
            }

            // Semaphore that will be signaled when when the first command buffer finish execution
            {
                VkSemaphoreCreateInfo command_buffer_finished_semaphore_create_info;
                command_buffer_finished_semaphore_create_info.flags = 0;
                command_buffer_finished_semaphore_create_info.pNext = nullptr;
                command_buffer_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                vkCreateSemaphore(render_context_->GetLogicalDeviceHandle(), &command_buffer_finished_semaphore_create_info, nullptr, &sync_primitives.render_finish_semaphore);
            }

            // Fence that will block until queue commands finished executing
            {
                VkFenceCreateInfo fence_create_info;
                fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                fence_create_info.pNext = nullptr;
                fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                vkCreateFence(render_context_->GetLogicalDeviceHandle(), &fence_create_info, nullptr, &sync_primitives.in_flight_fence);
            }

            syncrhonization_primitive_[i] = sync_primitives;
        }

        // TODO validations
        
        return true;
    }

}
